<p align="center">
<img src="http://i.imgur.com/mCtrbIN.png">
</p>
===================
ВНИМАНИЕ НЕ ВСЕ ПЕРЕМЕННЫЕ ЗАШИФРОВАНЫ!

# Обратите внимание что AimTux-Fuzion требует дополнительные пакеты. см. ниже!
[![Присоединяйтесь к чату на https://gitter.im/Aimtux-Fuzion/Lobby](https://img.shields.io/gitter/room/TechnologyAdvice/Stardust.svg?style=flat-square)](https://gitter.im/Aimtux-Fuzion/Lobby#)

Хотите общаться и играть с другими пользователями Fuzion? Присоединяйтесь к нам на Gitter!

## Что же такое Fuzion?

Это полнофункциональный встраиваемый чит для *CounterStrike : Global Offensive* написаный на C++.


## Сборка

**Примечание:** _Fuzion требует дополнительные пакеты для Lua и xdo._

**Примечание:** _НЕ СКАЧИВАЙТЕ И НЕ СОБИРАЙТЕ из под root!_

#### Скачайте требуемые зависимости для сборки Fuzion:


Если у вас возникли проблемы со сборкой, то вам нужена последняя версия(от 5,2) `g++`.

[Как обновить g++](https://github.com/AimTuxOfficial/AimTux/wiki/Updating-your-compiler)

==================

__Основаные на Ubuntu и Ubuntu / Debian:__
```bash
sudo apt-get install cmake g++ gdb git libsdl2-dev zlib1g-dev liblua5.3 libxdo-dev patchelf
```
__Arch:__
```bash
sudo pacman -S base-devel cmake gdb git sdl2 lua xdotool patchelf
```
__Fedora:__
```bash
sudo dnf install cmake gcc-c++ gdb git libstdc++-static mesa-libGL-devel SDL2-devel zlib-devel lua-devel libX11-devel libxdo-devel patchelf
```

__Gentoo:__
```bash
sudo emerge cmake dev-vcs/git gdb libsdl2 mesa lua xdotool patchelf
```
===================

#### Скачивание Fuzion:

```bash
git clone --recursive https://github.com/MegaRak64/simple_port.git
```

```bash
cd Fuzion
```

===================


#### Сборка с помощью скрипта

Вы можете собрать проще с встроенным скриптом сборки.
```bash
./build
```
Позже вы сможете обновить с помощью 
```bash
./update
```

## Инжектинг

Прежде чем начать, откройте CSGO , можно не заходить в игру . Однако, очень не рекомендуеться инжектить чит в CSGO во время загрузки карты. 

Перейдите в папку с AimTux-Fuzion где вы его собрали.

```bash
cd Fuzion
```

Теперь, вы можете инжектить чит с `load` скрипта

```bash
./load
```

Возможно, потребуеться ввести пароль, это связано с тем, что инжект требует доступа root.


Вывод в терминале во время инжекта не важен. 

Если инжект прошел успешно, вы увидите сообщение внизу «Successfully injected!», Однако, если сообщение говорит «Injection failed», то вы, скорее всего, сделали что-то неправильно.

Теперь вернитесь в csgo, если вы находитесь в главном меню игры, вы должны увидеть баннер в верхнем левом углу:

![this](http://i.imgur.com/Gb0SV1u.png)

## Предзагрузка(experimental)
У нас есть новый метод загрузки, который использует LD_PRELOAD и функцию подключения к библиотеке, чтобы оставаться незамеченной.

Чтобы использовать этот метод, вам необходимо закрыть CSGO и использовать сценарий предварительной загрузки с параметрами запуска CSGO.

Вот пример. Вам нужно написать *-steam* для включение VAC. 
```
./preload -steam -nobreakpad -high -threads 4
```

После загрузки CSGO чит должен так-же загрузиться, проверте консоль CSGO для получения дополнительной информации.

## Использование чита

После того как Fuzion был загружен, нажмите <kbd>Insert</kbd> что-бы открыть меню чита или (<kbd>ALT</kbd>+<kbd>I</kbd> если используете ноутбук).

Если вы хотите изменить скины, создать и загрузить конфигурации или открыть список игроков, вы можете найти эти кнопки в верхней части экрана.

## Выключение чита 

Если вы хотите выгрузить чит из игры, вы можете сделать это, введя команду:

```bash
./uload
```

## Настройка 

Конфигурации хранятся в скрытом каталоге в вашей домашней папке. конкретно в:

```
~/.config/Fuzion
```

Каждый `config.json` хранится в отдельной папке (имя папки соответствует тому которое вы видите в игре, в окне конфигурации).

Чтобы добавить конфигурацию, создайте папку внутри папки `~ / .config / Fuzion` с именем по вашему выбору и вставьте` config.json` внутри этой папки.

Что-бы увидить скрытые папки внутри вашей домешней директории, нажмите <kbd>CTRL</kbd>+<kbd>H</kbd> Когда используете файловый менеджер.

В терминале вы также можете добавить параметр -a в команду ls     `Пример: ls -la ~/` 

## Настройки гранат

```
~/.config/FuzionGH
```

Каждый `config.json` хранится в папке с именем после имени карты.

Чтобы добавить конфигурацию, скопируйте папку, содержащую ее, в `~ / .config / FuzionGH`

## Скриншоты
![franklin](http://i.imgur.com/a964edK.jpg)
![menu](http://i.imgur.com/AOSjO6S.jpg)
![swag](http://i.imgur.com/Okzvm5r.jpg)
![fuzion](http://i.imgur.com/5fZDVHa.jpg)

## Благодарности
Special thanks to the Original AimTux project: https://github.com/AimTuxOfficial/AimTux

Special thanks to [@aixxe](http://www.github.com/aixxe/) ([aixxe.net](http://www.aixxe.net)) for the skin changer and with the initial project, as well as helping this project with source code (Available on [@aixxe's](http://www.github.com/aixxe/) github page.)

This project was also originally based upon Atex's [Linux Basehook](http://unknowncheats.me/forum/counterstrike-global-offensive/181878-linux-basehook.html).

Перевели: [@MegaRak64] (http://www.github.com/MegaRak64/) [@NonStop] (http://www.github.com/NonStopez/)