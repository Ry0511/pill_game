//
// Date       : 24/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/game_renderer.h"

#include "SDL3/SDL.h"

namespace pill_game::game {

namespace {

SDL_AudioSpec device_audio_spec{};

void load_audio(void);

}  // namespace

AudioSource::~AudioSource() noexcept {
    if (Data) {
        PG_LOG(Trace, "Freeing audio data {:p}", static_cast<void*>(Data));
        SDL_free(Data);
    }
}

AudioSource::AudioSource(AudioSource&& other) noexcept {
    Position = std::exchange(other.Position, 0);
    SizeInBytes = std::exchange(other.SizeInBytes, 0);
    Data = std::exchange(other.Data, nullptr);
}

AudioSource& AudioSource::operator=(AudioSource&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    Position = std::exchange(other.Position, 0);
    SizeInBytes = std::exchange(other.SizeInBytes, 0);
    Data = std::exchange(other.Data, nullptr);
    return *this;
}

void init_audio(void) {
    device_audio_spec = SDL_AudioSpec{
        .format = SDL_AUDIO_F32,
        .channels = AUDIO_CHANNELS,
        .freq = AUDIO_FREQ,
    };

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &device_audio_spec
    );

    if (device == 0) {
        throw std::runtime_error{
            std::format("Failed to open audio device - {}", SDL_GetError())
        };
    }

    SDL_AudioStream* audio_stream = SDL_OpenAudioDeviceStream(
        device,
        &device_audio_spec,
        nullptr,
        nullptr
    );

    if (audio_stream == nullptr) {
        SDL_CloseAudioDevice(device);
        throw std::runtime_error{
            std::format("Failed to open audio device stream - {}", SDL_GetError())
        };
    }

    ctx().AudioDeviceId = device;
    ctx().AudioStream = audio_stream;
    SDL_ResumeAudioStreamDevice(ctx().AudioStream);
    SDL_SetAudioStreamGain(ctx().AudioStream, 0.1F);

    try {
        load_audio();
    } catch (const std::exception& ex) {
        SDL_DestroyAudioStream(audio_stream);
        SDL_CloseAudioDevice(device);
        throw ex;
    }
}

void tick_audio(void) noexcept {
    auto& audio_sources = ctx().AudioSources;
    SDL_AudioStream* stream = ctx().AudioStream;

    AudioSource& bgm = audio_sources.at(0);
    if (SDL_GetAudioStreamQueued(stream) < static_cast<int32_t>(bgm.SizeInBytes)) {
        PG_LOG(Info, "Queueing BGM data");
        SDL_PutAudioStreamData(stream, bgm.Data, static_cast<int32_t>(bgm.SizeInBytes));
    }
}

namespace {

void load_audio(void) {
    auto assets = std::filesystem::current_path() / "assets";

    auto load_audio = [](const fs::path& file, AudioSource& out) -> void {
        if (!fs::is_regular_file(file)) {
            return;
        }

        SDL_AudioSpec spec{};
        AudioSource file_src{};
        SDL_LoadWAV(
            file.string().c_str(),
            &spec,
            &file_src.Data,
            &file_src.SizeInBytes
        );

        uint8_t* data{nullptr};
        int32_t len{0};
        bool ok = SDL_ConvertAudioSamples(
            &spec,
            file_src.Data,
            static_cast<int32_t>(file_src.SizeInBytes),
            &device_audio_spec,
            &data,
            &len
        );

        if (!ok) {
            PG_LOG(Err, "Failed to resample audio file {} - {}", file.string(), SDL_GetError());
            if (data != nullptr) {
                SDL_free(data);
            }
            return;
        }

        out = std::move(AudioSource(0U, static_cast<uint32_t>(len), data));
    };

    auto& sources = ctx().AudioSources;
    load_audio(assets / "BG_01.wav", sources.at(0));
    load_audio(assets / "BG_02.wav", sources.at(1));
}

}  // namespace

}  // namespace pill_game::game
