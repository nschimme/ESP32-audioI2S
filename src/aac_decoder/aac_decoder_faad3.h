/*
 *  aac_decoder_faad3.h
 *  libfaad3 (faad3/) wrapper, selected with -DAAC_USE_FAAD3
 */

#pragma once

#include "../Audio.h"
#include "faad3/faad.h"

class AACDecoder : public Decoder {

  public:
    enum : int8_t {
        AAC_NEED_MORE_DATA = 101,
        AAC_ID3_HDR = 100,
        AAC_NONE = 0,
        AAC_ERR = -1,
    };

    AACDecoder(Audio& audioRef);
    ~AACDecoder() { reset(); }
    bool                  init() override;
    void                  clear() override;
    void                  reset() override;
    bool                  isValid() override;
    int32_t               findSyncWord(uint8_t* buf, int32_t nBytes) override;
    uint8_t               getChannels() override;
    uint32_t              getSampleRate() override;
    uint32_t              getOutputSamples();
    uint8_t               getBitsPerSample() override;
    uint32_t              getBitRate() override;
    uint32_t              getAudioDataStart() override;
    uint32_t              getAudioFileDuration() override;
    const char*           getStreamTitle() override;
    const char*           whoIsIt() override;
    int32_t               decode(uint8_t* inbuf, int32_t* bytesLeft, int32_t* outbuf) override;
    void                  setRawBlockParams(uint8_t channels, uint32_t sampleRate, uint8_t BPS, uint32_t tsis, uint32_t AuDaLength) override;
    std::vector<uint32_t> getMetadataBlockPicture() override;
    const char*           arg1() override;
    const char*           arg2() override;
    virtual int32_t       val1() override; // Parametric Stereo
    virtual int32_t       val2() override; // SBR

  private:
    bool openDecoder(const uint8_t* asc, uint32_t ascLen);
    void closeDecoder();

    Audio&           audio;
    faad_decoder*    m_dec = nullptr;
    void*            m_mem = nullptr; // the decoder's state, one block
    faad_frame_info  m_frameInfo = {};
    bool             m_f_decoderIsInit = false;
    bool             m_f_firstCall = false;
    bool             m_f_setRaWBlockParams = false;
    bool             m_f_adts = true;
    bool             m_f_framed = false; // a frame has been decoded since the decoder was opened
    uint32_t         m_aacSamplerate = 0;
    uint8_t          m_aacChannels = 0;
    uint8_t          m_aacProfile = 0;
    uint16_t         m_validSamples = 0;
    float            m_compressionRatio = 1;
};
