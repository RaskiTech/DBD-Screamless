# Screamless
Screamess is an extention program for Dead By Daylight that removes survivors from screaming when the killer hooks them.

It works by looking at the UI of the game and tabbing out at a precise time when the killer is about to hook a survivor. This makes Dead By Daylight stop the screaming sound. You can also do this manually, but this program automates that for you. Here is an on/off example video: (disable mute)

https://github.com/RaskiTech/DBD-Screamless/assets/51312660/72e588b8-7568-436c-91dc-88244cfe9ceb

I'm experimenting with an update that would completely mute the whole scream, so look forward to that.

# Limitations
You have to be on windows to run this program. I might consider adding linux support later.

Your game needs to be in Windowed or Borderless Windowed -mode. Tabbing out while in Fullscreen-mode is not ideal.

Your lobby has to have 4 survivors in it (alive or dead). It doesn't work in custom games with 1-3 survivors.

The program only mutes hook screams when you yourself are playing the killer. This is because I don't want people tabbing out randomly when playing survivor. You don't have to worry about this though, Screamless detects it automatically. Luckily the screams are only annoying when playing the killer.

# Will I Get Banned
Likely, no. This interferes with the game less than applications like Reshade do. Of course, I can't promise anything, but I don't know anybody who has gotten banned by using this application.

# Installation
Look for the releases tab and choose the latest release. Installation instructions are included in there.

# Troubleshooting
If you have any problems with the program, first check that your in-game hud scale is set correctly in the Settings.txt file. You should also check the program output by enabling console in Settings.txt. If everything is set up correctly, or the program throws weird errors you don't understand, open up an issue on the subject.

