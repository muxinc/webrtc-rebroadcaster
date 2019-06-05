/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 * 
 * Wraps FFmpeg stuff.
 */

#include "transcoder.h"

#include <iostream>

Transcoder::Transcoder(bool video, bool audio) :
    haveVideo(video), haveAudio(audio), frameNum(0) {
    std::cerr << "Created a transcoder with audio " << audio << " and video " << video << std::endl;
}

Transcoder::~Transcoder() {
    // Join threads
    if (this->outputContext) {
        avformat_free_context(this->outputContext);
    }
}

bool Transcoder::Start() {
    // TODO: Some thread stuff prolly

    if (avformat_alloc_output_context2(&this->outputContext, NULL, NULL, "/tmp/webrtc_dump.mp4") < 0) {
        std::cerr << "ERROR: Failed to alloc output context" << std::endl;
        return false;
    };

    if (this->haveVideo) {


        // av_opt_set(c->priv_data, "preset", "slow", 0);
    }

    return true;
}

void Transcoder::Stop() {
    this->shouldStop = true;

    // Condition variable stuff?
}

//
// VideoSinkInterface implementation
//
void Transcoder::OnFrame(const webrtc::VideoFrame &frame) {
    if (!this->haveVideo) {
        std::cerr << "ERROR: Received unexpected video frame" << std::endl;
    }

    if (!this->hadFirstFrame) {
        std::cerr << "Hey cool a frame" << std::endl;
        this->hadFirstFrame = true;

        // Ensure FFmpeg has been setup
        this->width = frame.width();
        this->height = frame.height();

        // Create a video transcode if it doesn't exist

        /* find the encoder */
        std::cerr << "Find encoder" << std::endl;
        this->vCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!(this->vCodec)) {
            std::cerr << "Could not find encoder for h264" << std::endl;
            return;
        }

        std::cerr << "New stream" << std::endl;
        this->vStream = avformat_new_stream(this->outputContext, NULL);
        if (!this->vStream) {
            std::cerr << "Could not allocate new stream" << std::endl;
            return;
        }
        this->vStream->id = this->outputContext->nb_streams - 1;

        std::cerr << "alloc context" << std::endl;
        this->vCodecCtx = avcodec_alloc_context3(this->vCodec);
        if (!this->vCodecCtx) {
            std::cerr << "Could not allocate new codec context" << std::endl;
        }

        this->vCodecCtx->codec_id = AV_CODEC_ID_H264;
        this->vCodecCtx->bit_rate = 4 * 1000 * 1000;
        this->vCodecCtx->width = frame.width();
        this->vCodecCtx->height = frame.height();

        this->vStream->time_base = (AVRational){ 1, 30 };
        this->vCodecCtx->time_base = this->vStream->time_base;
        this->vCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

        std::cerr << "avcodec open" << std::endl;
        AVDictionary *opt = NULL;
        int ret = avcodec_open2(this->vCodecCtx, this->vCodec, &opt);
        if (ret < 0) {
            char errBuf[1024];
            av_strerror(ret, errBuf, 1024);
            std::cerr << "Could not open video codec: " << errBuf << std::endl;
            return;
        }

        this->frame = av_frame_alloc();
        if (!this->frame) {
            std::cerr << "Failed to allocate frame" << std::endl;
            return;
        }

        this->frame->format = AV_PIX_FMT_YUV420P;
        this->frame->width = frame.width();
        this->frame->height = frame.height();
        ret = av_frame_get_buffer(this->frame, 32);
        if (ret < 0) {
            std::cerr << "Could not allocate frame data" << std::endl;
            return;
        }

        ret = avcodec_parameters_from_context(this->vStream->codecpar, this->vCodecCtx);
        if (ret < 0) {
            std::cerr << "COuld not copy the stream params" << std::endl;
            return;
        }


        if (!(this->outputContext->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&this->outputContext->pb, "/tmp/webrtc_dump.mp4", AVIO_FLAG_WRITE);
            if (ret < 0) {
                std::cerr << "Failed to open avio output" << std::endl;
                return;
            }
        }

        ret = avformat_write_header(this->outputContext, NULL);
        if (ret < 0) {
            std::cerr << "Failed to open output" << std::endl;
            return;
        }

        this->codecInitialized = true;
    }

    if (!this->codecInitialized) {
        return;
    }

    if (frame.width() != this->width || frame.height() != this->height) {
        std::cerr << "uh-oh, resolution changed" << std::endl;
        return;
    }

    rtc::scoped_refptr<webrtc::I420BufferInterface> i420Buf = frame.video_frame_buffer()->ToI420();

    std::cerr << "Encoding frame number: " << this->frameNum << std::endl;

    this->frame->pts = this->frameNum++;
    this->frame->data[0] = const_cast<uint8_t*>(i420Buf->DataY());
    this->frame->data[1] = const_cast<uint8_t*>(i420Buf->DataU()); 
    this->frame->data[2] = const_cast<uint8_t*>(i420Buf->DataV());

    int ret = avcodec_send_frame(this->vCodecCtx, this->frame);
    if (ret < 0) {
        std::cerr << "Failed to send frame" << std::endl;
    }

    std::cerr << "Frame encoded" << std::endl;

    while (1) {
        AVPacket *pkt = av_packet_alloc();
        ret = avcodec_receive_packet(this->vCodecCtx, pkt);
        std::cerr << "Received a packet" << std::endl;
        if (ret < 0) {
            if (ret == AVERROR(EAGAIN)) {
                break;
            }
            std::cerr << "ERROR: Failed to call receive packet: " << ret << std::endl;
            return;
        }

        std::cerr << "Writing frame" << std::endl;
        av_write_frame(this->outputContext, pkt);
        std::cerr << "Frame written" << std::endl;
    }

    if (this->frameNum > (30 * 60)) {
        this->codecInitialized = false;

        avcodec_send_frame(this->vCodecCtx, NULL);

        av_write_trailer(this->outputContext);
    }
}

//
// AudioTrackSinkInterface
//
void Transcoder::OnData(const void* audio_data, int bits_per_sample, int sample_rate, size_t number_of_channels, size_t number_of_frames) {
    if (!this->hadFirstAudio) {
        std::cerr << "hey cool audio data" << std::endl;

        this->hadFirstAudio = true;
    }
}