-- rocket_league.lua
-- example Modest Clipper profile for Rocket League
--
-- script activation itself is handled by Modest Clipper.
-- for example:
--   process: RocketLeague.exe
--   auto activate: on
--   require foreground: on
--   active while minimized: off
--
-- this file controls what happens while the script is active.
--
-- NOTE:
-- this is a proposed example API for the future Lua integration.
-- function/setting names may change during implementation.


-- =========================================================
-- profile lifecycle
-- =========================================================

function on_activate()
    print("rocket league profile activated")

    clipper.set("time", 40)
    clipper.set("fps", 60)

    clipper.set("encoder", "auto")
    clipper.set("codec", "h264")
    clipper.set("bitrate", 20000)

    clipper.set("desktop_audio", true)
    clipper.set("microphone", false)

    clipper.set("container", "mp4")
    clipper.set("output_folder", "Rocket League")

    clipper.set("show_save_notification", true)
    clipper.set("play_save_sound", true)
end


function on_deactivate()
    print("rocket league profile deactivated")
end


-- =========================================================
-- clip events
-- =========================================================

function on_clip_saved(path)
    print("rocket league clip saved: " .. path)
end


function on_clip_failed(error)
    print("failed to save rocket league clip: " .. error)
end


-- =========================================================
-- basic capture options
-- =========================================================

-- clipper.set("time", 15)
-- clipper.set("time", 30)
-- clipper.set("time", 40)
-- clipper.set("time", 60)
-- clipper.set("time", 120)

-- clipper.set("fps", 30)
-- clipper.set("fps", 60)
-- clipper.set("fps", 120)

-- clipper.set("monitor", 1)
-- clipper.set("capture", true)


-- =========================================================
-- encoding
-- =========================================================

-- clipper.set("encoder", "auto")
-- clipper.set("encoder", "nvenc")
-- clipper.set("encoder", "amf")
-- clipper.set("encoder", "qsv")

-- clipper.set("codec", "h264")
-- clipper.set("codec", "hevc")
-- clipper.set("codec", "av1")

-- clipper.set("bitrate", 12000)
-- clipper.set("bitrate", 20000)
-- clipper.set("bitrate", 30000)
-- clipper.set("bitrate", 50000)

-- clipper.set("rate_control", "cbr")
-- clipper.set("rate_control", "vbr")
-- clipper.set("rate_control", "cq")

-- clipper.set("preset", "performance")
-- clipper.set("preset", "balanced")
-- clipper.set("preset", "quality")

-- clipper.set("keyframe_interval", 2)
-- clipper.set("b_frames", 2)


-- =========================================================
-- resolution
-- =========================================================

-- clipper.set("resolution", "native")
-- clipper.set("resolution", "1080p")
-- clipper.set("resolution", "1440p")
-- clipper.set("resolution", "4k")

-- clipper.set("width", 1920)
-- clipper.set("height", 1080)

-- clipper.set("scale", 1.0)
-- clipper.set("keep_aspect_ratio", true)


-- =========================================================
-- audio
-- =========================================================

-- clipper.set("desktop_audio", true)
-- clipper.set("microphone", true)

-- clipper.set("desktop_device", "default")
-- clipper.set("microphone_device", "default")

-- clipper.set("desktop_volume", 1.0)
-- clipper.set("microphone_volume", 0.85)

-- clipper.set("audio_bitrate", 192)
-- clipper.set("audio_sample_rate", 48000)
-- clipper.set("audio_channels", 2)


-- =========================================================
-- replay buffer
-- =========================================================

-- clipper.set("buffer_time", 20)
-- clipper.set("buffer_time", 40)
-- clipper.set("buffer_time", 60)
-- clipper.set("buffer_time", 120)

-- clipper.set("max_buffer_memory", 512)
-- clipper.set("trim_old_packets", true)


-- =========================================================
-- output
-- =========================================================

-- clipper.set("output", "D:/Clips/Rocket League")
-- clipper.set("container", "mp4")
-- clipper.set("container", "mkv")

-- clipper.set("filename", "rl_{date}_{time}")
-- clipper.set("filename", "{game}_{date}_{time}")

-- clipper.set("create_game_folders", true)
-- clipper.set("overwrite_existing", false)


-- =========================================================
-- notifications
-- =========================================================

-- clipper.set("show_save_notification", true)
-- clipper.set("play_save_sound", true)
-- clipper.set("save_sound", "saved.wav")
-- clipper.set("show_errors", true)


-- =========================================================
-- custom hotkeys
-- =========================================================

-- clipper.bind("ctrl+shift+f7", function()
--     clipper.capture()
-- end)

-- clipper.bind("ctrl+shift+f8", function()
--     clipper.set("time", 15)
--     clipper.capture()
-- end)

-- clipper.bind("ctrl+shift+f9", function()
--     clipper.set("time", 60)
--     clipper.capture()
-- end)


-- =========================================================
-- foreground / minimized behavior
-- =========================================================
--
-- whether this script activates while Rocket League is opened,
-- focused, minimized, or in the background should normally be
-- configured by Modest Clipper itself.
--
-- these callbacks can optionally change behavior after the
-- application reports a state change.

-- function on_focus()
--     clipper.set("fps", 60)
--     clipper.resume()
-- end

-- function on_blur()
--     print("rocket league lost focus")
-- end

-- function on_minimize()
--     clipper.pause()
-- end

-- function on_restore()
--     clipper.resume()
-- end


-- =========================================================
-- reading current settings
-- =========================================================

-- local time = clipper.get("time")
-- local fps = clipper.get("fps")
-- local codec = clipper.get("codec")

-- print("time: " .. time)
-- print("fps: " .. fps)
-- print("codec: " .. codec)


-- =========================================================
-- manual commands
-- =========================================================

-- clipper.capture()
-- clipper.pause()
-- clipper.resume()
-- clipper.reload()
-- clipper.status()
