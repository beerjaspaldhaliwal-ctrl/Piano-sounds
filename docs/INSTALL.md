# Installing Keyverse

Keyverse comes as a **VST3** plug-in (all DAWs except Logic/GarageBand), an **AU** plug-in
(macOS: Logic, GarageBand), an **LV2** plug-in (Linux) and a **Standalone** app that runs
without a DAW.

After copying the files, open your DAW and rescan plug-ins. Keyverse appears as an
instrument from "Keyverse Audio".

## Windows

1. Copy the `Keyverse.vst3` folder to `C:\Program Files\Common Files\VST3\`.
2. Optional: put `Keyverse.exe` (the standalone app) anywhere you like and run it.
3. If Windows SmartScreen warns about the app, click **More info** and then **Run anyway**
   (the build is not code-signed).

## macOS

1. Double-click `Keyverse-macOS.zip` if your browser did not already unzip it, so you have a
   `Keyverse` folder.
2. Copy `Keyverse.vst3` to `~/Library/Audio/Plug-Ins/VST3/`.
3. Copy `Keyverse.component` to `~/Library/Audio/Plug-Ins/Components/` (for Logic / GarageBand).
4. Copy `Keyverse.app` to `/Applications` (optional standalone app).
5. The build is not notarised, so macOS quarantines it. Open **Terminal** and run:

   ```
   xattr -cr ~/Library/Audio/Plug-Ins/VST3/Keyverse.vst3
   xattr -cr ~/Library/Audio/Plug-Ins/Components/Keyverse.component
   xattr -cr /Applications/Keyverse.app
   ```

6. For Logic, restart it (or run `killall -9 AudioComponentRegistrar` in Terminal) so it
   picks up the new AU.

## Linux

1. Extract the archive: `tar xzf Keyverse-Linux.tar.gz`.
2. Copy `Keyverse.vst3` to `~/.vst3/` and `Keyverse.lv2` to `~/.lv2/`.
3. Run `./Keyverse` for the standalone app.

## First steps

- Pick a category on the left (pianos and synth keys are at the top) and click a sound.
- Turn the **Brightness**, **Timbre**, **Time** and **Movement** knobs to shape it.
- Click **Edit** to change any detail, then **Save** to keep your own version.
