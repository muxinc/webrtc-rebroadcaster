/**
 * Copyright 2019 Nick Chadwick <nick@mux.com>. All rights reserved.
 * 
 * Do some transcoding and stuff.
 */

#ifndef TRANSCODER_H_
#define TRANSCODER_H_

#include <api/media_stream_interface.h>
#include <api/video/video_frame.h>
#include <api/video/video_sink_interface.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class Transcoder : public webrtc::AudioTrackSinkInterface,
                   public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    Transcoder(bool video, bool audio);
    virtual ~Transcoder();

    bool Start();
    void Stop();

    //
    // VideoSinkInterface implementation
    //
    void OnFrame(const webrtc::VideoFrame &frame) override;

    //
    // AudioTrackSinkInterface
    //
    void OnData(const void* audio_data, int bits_per_sample, int sample_rate, size_t number_of_channels, size_t number_of_frames) override;

private:
    bool shouldStop;
    bool hadFirstFrame;
    bool hadFirstAudio;
    bool haveVideo;
    bool haveAudio;

    int width;
    int height;

    AVFormatContext* outputContext;

    // Video Stuff
    AVCodec *vCodec;
    AVCodecContext *vCodecCtx;
    AVStream *vStream;

    // Audio Stuff
};

#endif // TRANSCODER_H_