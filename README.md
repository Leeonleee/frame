# frame

> An exposure logger for film photographers, on your wrist.

`frame` is a [Pebble](https://repebble.com) watchapp that records the exposure
settings you used for every photo you take on film.
Film gives you no EXIF data, so it is easy to forget what shutter speed,
aperture, and ISO you metered for by the time the roll comes back from the lab.
`frame` keeps a running log per roll and per frame, right on your watch, so you
can note your settings the moment you take the shot and refer back to them
later.

It is built for the **Pebble Time 2** (`emery`) and also runs on the other
Pebble platforms.

## Features

- Track multiple rolls of film at once.
- Pick from a built-in list of common film stocks, grouped by brand
  (Kodak, Ilford, Fujifilm, CineStill).
- Log shutter speed, aperture, and ISO for each frame.
- ISO defaults to the film's box speed and is adjustable for push/pull
  processing.
- New frames carry over the previous frame's settings, so logging a series is
  fast.
- Everything is saved to the watch's persistent storage and survives the app
  closing.

## Controls

`frame` follows the standard Pebble button layout: **Back** on the left,
**Up** / **Select** / **Down** on the right.

| Screen | Up / Down | Select | Hold Select | Back |
|--------|-----------|--------|-------------|------|
| Roll List | Move selection | Open roll, or start a new one on `+ New Roll` | Delete the highlighted roll (with confirmation) | Exit app |
| Stock Picker | Move selection | Create a roll with this stock | - | Cancel |
| Roll screen | Move selection | Add a frame, or edit the highlighted one | Delete the highlighted frame (with confirmation) | Back to Roll List |
| Frame Editor | Change the highlighted value | Move to the next field | Save the frame | Discard changes |

## Screens

- **Roll List** is the home screen.
  It lists every roll, newest first, with a `+ New Roll` row at the bottom.
- **Stock Picker** lets you choose a film stock to start a new roll.
- **Roll screen** shows a roll's frames, newest first, with `+ Add Frame` at the
  top for quick access.
- **Frame Editor** is where you set shutter, aperture, and ISO.
  Up and Down change the highlighted value, Select cycles through the fields,
  and holding Select saves.

## Building and running

You will need the [Pebble SDK](https://developer.repebble.com) and its `pebble`
command line tool.

```sh
pebble build                          # build for all target platforms
pebble install --emulator emery       # run in the Pebble Time 2 emulator
```

Add `--logs` to an `install` command to stream the app's logs.

### Target platforms

`targetPlatforms` in `package.json` controls which watches you build for.
The modern Pebble hardware is `emery` (Pebble Time 2), `gabbro` (Pebble Round
2), and `flint` (Pebble 2 Duo).
The original Pebble platforms (`aplite`, `basalt`, `chalk`, `diorite`) are
included for backwards compatibility.

## Project structure

```
src/c/frame.c           App entry point; loads data and pushes the Roll List
src/c/data.{c,h}         Data model, hardcoded tables, and persistence
src/c/roll_list.{c,h}    Roll List screen (home)
src/c/stock_picker.{c,h} Film-stock chooser for a new roll
src/c/roll_window.{c,h}  A single roll's frame list
src/c/frame_editor.{c,h} Shutter / aperture / ISO editor
src/c/confirm.{c,h}      Reusable delete-confirmation dialog
package.json             Project metadata (UUID, platforms, resources)
wscript                  Build rules
```

## Data model

Each setting is stored as a small index into a fixed table, so a frame is only
three bytes and a whole roll fits in a single persistent-storage value.
Rolls are stored oldest-first and presented newest-first.
Persistence lives entirely behind the data layer in `data.c`, so the screen code
never touches the `persist_*` API directly.

## To-do

- [ ] Polish the Frame Editor layout on the round display (`chalk`).
- [ ] Add an `ActionBarLayer` with icons to make the editor controls more
      discoverable.
- [ ] In-app settings screen.
- [ ] Export a roll as CSV (needs a PebbleKit JS companion on the phone).
- [ ] Custom / other film stock with a manual ISO.
- [ ] Optional per-frame notes (lens, focal length, exposure compensation).
- [ ] Optional 24 / 36 exposure count per roll.
