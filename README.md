![Static Badge](https://img.shields.io/badge/Language-C-lightgrey) ![Static Badge](https://img.shields.io/badge/Library-Concord-purple) ![Static Badge](https://img.shields.io/badge/Library-CogLink-purple)

# World [ADO](https://x.com/ado1024imokenp) Domiation!

<img width="200" src="https://media.tenor.com/yIX_27rQAIkAAAAM/ado-ado-cute.gif">

# Limited TNiC Bot

Limited TNiC is a full rewrite of nEXT bot with saving functionality. TNiC stands for "The nEXT is C". TNiC is fully compatible with Limited nEXT PostgreSQL DB, so you don't want to replace it.

> [!WARNING]  
> ### TNiC pre-alpha
> Bot is in the pre-alpha, so it has a small part of nEXT functionality.

## About

Limited nEXT is one of the bots in the 'Limited' series, which includes:

- [Limited TNiC](https://github.com/at-elcapitan/Limited-TNiC) (this bot) replaced [Limited nEXT](https://github.com/at-elcapitan/Limited_Py)
- [Limited C/Link](https://github.com/at-elcapitan/Limited-C_Link)
- [Limited jEXT](https://github.com/at-elcapitan/AT-Limited_jEXT)

The bot is designed to provide a seamless music streaming experience on Discord, equipped with features that make it stand out.

## Bot Setup and Usage

### Dependencies
- [Concord](https://github.com/Cogmasters/concord)
- [CogLink](https://github.com/PerformanC/CogLink)
- [rxi/map](https://github.com/rxi/map)

#### Installing build dependencies

Debian GNU/Linux
```bash
sudo apt install meson gcc gcc-multilib libcurl4-openssl-dev git
```

Arch Linux
```bash
sudo pacman -Sy meson gcc git curl
```

#### Installing Concord

Debian GNU/Linux
```bash
git clone https://github.com/cogmasters/concord.git && cd concord && make && sudo make install
```

Arch Linux
```bash
git clone https://aur.archlinux.org/concord-git.git
cd concord-git
makepkg -Acs
pacman -U concord-git-version-any.pkg.tar.zst
```
or
```bash
yay -S concord-git
```

#### Installing CogLink

```bash
git clone https://github.com/PerformanC/CogLink && cd CogLink && make && sudo make install
```

### Building bot

```bash
meson build --buildtype debug && cd build && ninja
```

### Configuring

Create config.json in `build` folder (or any another folder with bot's executable)

```
{
  "logging": { // logging directives
    "level": "trace",        // trace, debug, info, warn, error, fatal
    "filename": "bot.log",   // the log output file
    "quiet": false,          // change to true to disable logs in console
    "overwrite": true,       // overwrite file if already exists, append otherwise
    "use_color": true,       // display color for log entries
    "http": {
      "enable": true,        // generate http specific logging
      "filename": "http.log" // the HTTP log output file
    },
    "disable_modules": ["WEBSOCKETS", "USER_AGENT"] // disable logging for these modules
  },
  "discord": { // discord directives
    "token": "YOUR-BOT-TOKEN",         // replace with your bot token
    "default_prefix": {                 
      "enable": false,                 // enable default command prefix
      "prefix": "YOUR-COMMANDS-PREFIX" // replace with your prefix
    }
  },
  "bot_id" : 1234567,              // Replace with your bot ID
  "game_name" : "Your game",       // Not required. Setup if you want your bot to play some game
  "status" : "online",             // Required if game_name is defined
  "admin_debug_id" : 1234567       // Not required for work. Replace with your ID to allow commands only for your user
} 
```

## Commands

| Command Name      | Description                                        |
| ----------------- | -------------------------------------------------- |
| /youtube          | Play a YouTube track                               |
| /disconnect       | Disconnect bot from voice channel                  |

## Using external PostgreSQL Database

> Database connection not implemented
