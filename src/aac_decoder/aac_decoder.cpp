/*
 *  aac_decoder.cpp
 *  libfaad3 (faad3/) wrapper
 */

#include "aac_decoder.h"
#include "Arduino.h"
#include "esp_heap_caps.h"

#define AAC_LOG_ERROR(fmt, ...) Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define AAC_LOG_WARN(fmt, ...)  Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
AACDecoder::AACDecoder(Audio& audioRef) : Decoder(audioRef), audio(audioRef) {}

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool AACDecoder::init() { // the decoder is opened by the first decode(), when the stream format is known
    m_f_decoderIsInit = true;
    m_f_framed = false;
    m_f_firstCall = false;
    m_f_setRaWBlockParams = false;
    return m_f_decoderIsInit;
}
void AACDecoder::clear() {
    return; // nothing todo
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void AACDecoder::closeDecoder() {
    if (m_dec) faad_decoder_close(&m_dec);
    if (m_mem) heap_caps_free(m_mem);
    m_dec = nullptr;
    m_mem = nullptr;
}
void AACDecoder::reset() {
    closeDecoder();
    m_f_decoderIsInit = false;
    m_f_firstCall = false;
    m_f_framed = false;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool AACDecoder::openDecoder(const uint8_t* asc, uint32_t ascLen) {
    faad_config cfg;
    faad_config_init(&cfg, sizeof(cfg));
    cfg.stream_format = m_f_adts ? FAAD_STREAM_ADTS : FAAD_STREAM_RAW;
    cfg.output_format = FAAD_OUTPUT_32BIT;

    uint32_t bytes = 0;
    if (faad_get_state_size(&cfg, &bytes) != FAAD_OK) return false;
    m_mem = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!m_mem) m_mem = heap_caps_malloc(bytes, MALLOC_CAP_8BIT);
    if (!m_mem) return false;
    if (faad_decoder_init(m_mem, bytes, &cfg, asc, ascLen, &m_dec) != FAAD_OK) {
        closeDecoder();
        return false;
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool AACDecoder::isValid() {
    return m_f_decoderIsInit;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t AACDecoder::findSyncWord(uint8_t* buf, int32_t nBytes) {
    const int MIN_ADTS_HEADER_SIZE = 7;
    if (buf == nullptr || nBytes < MIN_ADTS_HEADER_SIZE) { return -1; }

    auto validate = [MIN_ADTS_HEADER_SIZE](const uint8_t* buf, int32_t bytesAvailable) -> bool { // check the ADTS header for validity
        if (bytesAvailable < MIN_ADTS_HEADER_SIZE) { return false; }

        // Layer (bits 14-15) must be 00
        if ((buf[1] & 0x06) != 0x00) { return false; }

        // Sampling Frequency Index (Bits 18-21) cannot be invalid
        uint8_t sampling_frequency_index = (buf[2] & 0x3C) >> 2;
        if (sampling_frequency_index > 12) { return false; }

        // Frame length (bits 30-42) must be at least the header size
        int frame_length = ((buf[3] & 0x03) << 11) | (buf[4] << 3) | ((buf[5] & 0xE0) >> 5);
        if (frame_length < MIN_ADTS_HEADER_SIZE) { return false; }

        return true;
    };

    /* find byte-aligned syncword (12 bits = 0xFFF) */
    for (int32_t i = 0; i <= nBytes - MIN_ADTS_HEADER_SIZE; i++) {
        if (buf[i + 0] == 0xff && (buf[i + 1] & 0xf0) == 0xf0) {
            int32_t bytesAvailable = nBytes - i;
            if (!validate(&buf[i], bytesAvailable)) { continue; }

            int frame_length = ((buf[i + 3] & 0x03) << 11) | (buf[i + 4] << 3) | ((buf[i + 5] & 0xE0) >> 5);
            if (i + frame_length + 1 >= nBytes) {
                return -1; // beyond the buffer, no valid header
            }
            /* find a second byte-aligned syncword (12 bits = 0xFFF) */
            if (buf[i + frame_length + 0] == 0xff && (buf[i + frame_length + 1] & 0xf0) == 0xf0) { return i; }
        }
    }

    return -1;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t AACDecoder::getChannels() {
    return m_aacChannels;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint32_t AACDecoder::getSampleRate() {
    return m_aacSamplerate;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint32_t AACDecoder::getOutputSamples() {
    return m_validSamples;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t AACDecoder::getBitsPerSample() {
    return 16;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint32_t AACDecoder::getBitRate() {
    uint32_t br = getBitsPerSample() * getChannels() * getSampleRate();
    return (br / m_compressionRatio);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint32_t AACDecoder::getAudioDataStart() {
    return 0; // nothing todo
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint32_t AACDecoder::getAudioFileDuration() {
    return 0; // nothing todo
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* AACDecoder::getStreamTitle() {
    return nullptr; // nothing todo
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* AACDecoder::whoIsIt() {
    return "AAC";
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t AACDecoder::decode(uint8_t* inbuf, int32_t* bytesLeft, int32_t* outbuf) {

    if (*bytesLeft >= 10 && inbuf[0] == 'I' && inbuf[1] == 'D' && inbuf[2] == '3') return AAC_ID3_HDR; // the caller skips the tag

    if (m_f_firstCall == false) {
        m_f_adts = !m_f_setRaWBlockParams;
        bool ok;
        if (m_f_setRaWBlockParams) { // raw AAC from an M4A: the ASC is rebuilt from the container's values
            m_f_setRaWBlockParams = false;
            static const uint32_t rates[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
            uint8_t               srIdx = 4;
            for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
                if (rates[i] == m_aacSamplerate) srIdx = i;
            }
            // only AAC-LC is decoded; SBR is found from the stream itself
            uint8_t asc[2] = {(uint8_t)((2 << 3) | (srIdx >> 1)), (uint8_t)((srIdx << 7) | (m_aacChannels << 3))};
            ok = openDecoder(asc, sizeof(asc));
        } else {
            ok = openDecoder(nullptr, 0);
        }
        if (!ok) return -100; // serious error, stop
        m_f_firstCall = true;
    }

    uint32_t consumed = 0, written = 0;
    faad_status st = faad_decode_frame(m_dec, inbuf, *bytesLeft, &consumed, outbuf, 4608 * 2 * sizeof(int32_t), &written, &m_frameInfo);
    *bytesLeft -= consumed;
    if (st == FAAD_ERR_NEED_MORE_DATA) return AAC_NEED_MORE_DATA;
    if (st != FAAD_OK) {
        AAC_LOG_WARN("{}", faad_strerror(st));
        return st == FAAD_ERR_SYNC_LOST ? AAC_ERR : (int32_t)st;
    }

    uint32_t channels = m_frameInfo.channels;
    uint32_t frames = written / (sizeof(int32_t) * channels);
    if (m_f_framed && channels != m_aacChannels) return -21; // mono <-> stereo change, the caller reopens the decoder
    m_f_framed = true;
    m_aacChannels = channels;
    m_aacSamplerate = m_frameInfo.sample_rate;
    m_validSamples = frames * channels;
    m_compressionRatio = (float)m_validSamples * 2 / (consumed ? consumed : 1);
    if (channels == 1) { // the I2S path is stereo
        for (int32_t i = frames - 1; i >= 0; i--) {
            int32_t sample = outbuf[i];
            outbuf[i * 2] = sample;
            outbuf[i * 2 + 1] = sample;
        }
    }
    return 0;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void AACDecoder::setRawBlockParams(uint8_t channels, uint32_t sampleRate, uint8_t dummy1, uint32_t profile, uint32_t dummy2) {
    m_f_setRaWBlockParams = true;
    m_aacChannels = channels;     // 1: Mono, 2: Stereo
    m_aacSamplerate = sampleRate; // 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
    m_aacProfile = profile;
    return;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
std::vector<uint32_t> AACDecoder::getMetadataBlockPicture() {
    std::vector<uint32_t> a;
    return a;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* AACDecoder::arg1() { // AAC format
    return m_f_adts ? "AAC HeaderFormat: ADTS" : "AAC HeaderFormat: RAW";
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* AACDecoder::arg2() {
    return m_frameInfo.sbr_active ? "SBR" : "without SBR";
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t AACDecoder::val1() { // Parametric Stereo
    return m_frameInfo.ps_active;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t AACDecoder::val2() { // Spectral Band Replication
    return m_frameInfo.sbr_active;
}
