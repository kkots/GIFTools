# Kkots GIFTools

## Description

Provides command-line utilities for Windows and Linux to perform following tasks:

- Given a set of files named file1.png, file2.png, file3.png and so on, renumber them to, for example, file2.png, file3.png, file4.png;
- Given a GIF file print or modify its frames durations (careful: not all GIF files may be supported);
- Given a set of files named file1.png, file2.png, file3.png and so on, delete every second file and renumber the rest so that they are numbered consecutively.

## Usage

Tools are meant to be used from the command line. On Windows you open the command line by:

- Pressing Win+R and typing `cmd`, then pressing Enter;
- Or pressing Win key, typing `Command Prompt` into the search, then pressing Enter;
- Other ways.

There are two way to speed up copying file paths into the Windows command prompt:

- Way 1:
  - Select the file in Windows Explorer;
  - Copy the file using Ctrl+C shortcut;
  - Press Win+R, which opens the `Run` prompt;
  - Paste the file into the `Run` prompt using Ctrl+V shortcut;
  - The full file path gets pasted with the quotation marks;
  - You can now copy this file path and paste it elsewhere.
- Way 2:
  - Drag and drop the file from the Windows Explorer window into the Command Prompt window.

### renumber_frames usage

renumber_frames is the command that does this:

- Given a set of files named file1.png, file2.png, file3.png and so on, renumber them to, for example, file2.png, file3.png, file4.png;

Example usage:

```cmd
D:\source\repos\GIFTools\Release\renumber_files.exe "D:\source\repos\GIFTools\screens\screen%.png" 48-58 49
```

The above example shifts frames 48-58 forward one frame, so they now become 49-59.

General syntax is:

```cmd
D:\source\repos\GIFTools\Release\renumber_files.exe <path> <start>-<end> <destination>
```

- `path` - the path to the frame file, with the number part replaces with `%` signs. You can increase the number of `%` signs if you want the number to be padded with zeros on the left;
- `start` - this is the starting frame number to move;
- `end` - this is the ending frame number to move, inclusive;
- `destination` - this is the destination frame number to which the start frame would get moved. The `start+1` frame would get moved into `destination+1` and so on;

### change_gif_durations

change_gif_durations is the command that does this:

- Given a GIF file print or modify its frames durations (careful: not all GIF files may be supported);

### Printing GIF frame durations

Example usage:

```cmd
D:\source\repos\GIFTools\Release\change_gif_durations.exe D:\source\repos\GIFTools\screens\out.gif -f > "D:\source\repos\GIFTools\screens\durations.txt"
```

The above example prints the duration of each frame of the GIF into the `durations.txt` text file that gets created. The duration is in milliseconds, and each frame duration is represented by a number on a new line.

General syntax is:

```cmd
D:\source\repos\GIFTools\Release\change_gif_durations.exe [path] -f > [output]
```

- `path` - the path to the GIF file;
- `output` - the path to the text file that will be created and will contain the durations.

The command also supports a more human-readable output, using the `-u` switch. Example:

```cmd
D:\source\repos\GIFTools\Release\change_gif_durations.exe D:\source\repos\GIFTools\screens\out.gif -f -u
```

Example output:

```text
1-13: 50 ms (20 fps)
14: 2000 ms (1 fps)
15-57: 50 ms (20 fps)
Average duration: 84 ms
Average framerate: 12 fps
Finished successfully.
```

### Changing GIF frame durations using -duration

Example usage:

```cmd
D:\source\repos\GIFTools\Release\change_gif_durations.exe D:\source\repos\GIFTools\screens\out.gif 0-20 -duration 1000
```

The above example sets durations for frames 0-20 to 1000 milliseconds (or 1 second) each. Frames are labeled starting from 0.

### Changing GIF frame durations using -fps

Example usage:

```cmd
D:\source\repos\GIFTools\Release\change_gif_durations.exe D:\source\repos\GIFTools\screens\out.gif 0-20 -fps 30
```

The above example sets the framerate for frames 0-20 to 30 frames per second. The rest of the frames are unaffected. Frames are labeled starting from 0.  
This is actually the same as using `-duration 33` (because `33 = 1000 / 30`).

### Changing GIF frame durations using -durations

You can use a text file which specifies duration for each frame in order to set GIF frame durations. Such text file must contain a frame duration in milliseconds on each line for all GIF frames. You're not allowed to specify a frame range in this approach. The durations text file can be exported from the GIF file by using the `-f` option (described in previous sections).

Example usage:

```cmd
D:\source\repos\GIFTools\Release\change_gif_durations.exe D:\source\repos\GIFTools\screens\out.gif -durations D:\source\repos\GIFTools\screens\durations.txt
```

This will set each frame's duration to the corresponding value in `durations.txt`.
