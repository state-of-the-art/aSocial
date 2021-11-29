# aSocial

[![Join the chat at https://gitter.im/state-of-the-art/aSocial](https://badges.gitter.im/state-of-the-art/aSocial.svg)](https://gitter.im/state-of-the-art/aSocial?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Open source private distributed social network application with focus on security, control and local store of social
information.

* [Site page about it](https://www.state-of-the-art.io/projects/asocial/)

## About

The general purpose of the application is to replace all the centralized social networks &
messengers and allow the users to never store their information ini just one place. aSocial was born
from the idea of complete control over social information, liberated from spam/ad, simplify
connection between people and fight censorship. Storing the information on the user's devices, using
strong cryptography with an ability of plausible deniability, integration the Bitcoin network will
allow us to solve the issues we see everyday in the huge centralized social providers.

### History

The project was started as typical social 12/28/2013 and mutated to distributed and found itself 10/17/2014. The POC
was focused on UI, but some basic components were also prepared. Stopped due to breaking Qt changes, not ready
Lightning network (second layer of Bitcoin for microtransactions) & and overall project complexity was stopped
03/30/2016.

The lessons have been learned and rerun the aSocial development was started with SDD and searching for people who can
understand & help with thinking and development.

## Usage

**WARNING:** By using this software you agree not to limit forcefully the other's freedom of
ditributing any kind of information even if you don't like it's nature. In case you can't comply -
please remove aSocial from your device as fast as possible.

Please check out the wiki page: [Wiki](https://github.com/state-of-the-art/asocial/wiki)

## Requirements

TODO

## Application

The application itself is native divided into 2 parts - background backend and UI frontend. Backend working with
a simple key-value database, frontend working with encrypted sql database to process profiles data.

### Core features

Basic features of the social platform:

* *Plugins* - add or remove components or use the other vendor component as a plugin
* *History* - store not just dates of item, but also any edit of the items
* *Persons* - personal & legal entities, existing (with profiles) or imaginary
* *Events* - anything happened, happening, going to happen in the life
* *Messages* - monetized conversation threads or any other interaction like profile updates, news
* *Overlays* - your vision or additional information for existing or new entities, could be password protected
* *Synchronization* - make sure your devices replicates the information / profiles you have
* *Sharing* - ability to share your files with friends from any device you have

### Additional features

Based on core features aSocial support the embedded and external additional features:

* *Tree of life* - usual thing, your relatives and relations
* *Testament* - special message, will be triggered to send on some condition
* *Contracts* - special message, signed by 2 parties, could have penalties, money and end dates
* *Web-version* - access to web interface from web browser

### Plans

You can see all the feature requests/bugs on the github page:

* [Milestones](https://github.com/state-of-the-art/asocial/milestones)
* [Issues](https://github.com/state-of-the-art/asocial/issues)

## OpenSource

This is an experimental project - the main goal is to test State Of The Art philosophy on practice.

We would like to see a number of independent developers working on the same project issues
for the real money (attached to the ticket) or just for fun. So let's see how this will work.

### License

Repository and it's content is covered by `Apache v2.0` - so anyone can use it without any concerns.

If you will have some time - it will be great to see your changes merged to the original repository -
but it's your choice, no pressure.

### Build

#### Build in docker

1. Clone the repository:
    ```
    $ git clone https://github.com/state-of-the-art/asocial.git && mkdir -p out
    ```

TODO

## Privacy policy

The whole purpose of this project - is to make sure your personal information will be controlled by
you, there is no way the aSocial (except for external plugins, please see their privacy policy) will
share your information without your permission with someone. But please be careful - once shared
information will stay on the Internet forever, so think twice about what you are doing.
