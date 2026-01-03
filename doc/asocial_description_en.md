# aSocial project description

## Executive Abstract

> **aSocial**: "Share your data with the ones you trust the most"

The modern world is a collection of centralized social networks and instant messengers which are
happy to own your social information without any restrictions on its actual use. This project was
created to solve the unpleasant problems of centralizing the storage of important information,
fight censorship, simplify communication and interaction of people, and also protect them from
outside encroachments.

**aSocial** is a distributed, private, and secure social network for those who care about freedom
of communication and want complete control over their social information. Imagine a world where
your family history lives forever on your terms, free from corporate overreach.

Key features include:
- Secure communication channels for interaction
- Storage of generational memories on user-controlled devices
- Payment-based filtering to combat spam and useless information
- Simplified search for new acquaintances
- Framework for building reliable and secure social applications

Naturally, the source code will be completely open to the public. In today's environment, it is
impossible to trust closed products and algorithms with which they handle our data. The platform
will be based on peer-to-peer communication technologies, eliminating central servers - which
significantly improves security and censorship resistance beyond what modern communication systems
can achieve.

The system will also provide:

- Information monetization through Bitcoin and Lightning networks
- Simplified user interfaces
- Streamlined content distribution
- Unified and protected storage and processing of social information
- Flexible historical data interaction

In general, it will give users full control over what, how, and when events occurred, are
occurring, and will occur in their lives, as well as those of their friends, relatives, and other
social contacts.

# 1. Project overview

## 1.1. Purpose

The main goal of the project is to create a distributed and secure platform for user interaction in
any form without restrictions.

Freedom and privacy of interaction are the most valuable good for the individual and it should be
strived for, and it cannot be subject to any restrictions. Even if this communication may seem
"harmful" to someone, this is only a private judgment and everyone is free to choose their own
destiny in connection with the circle of their communication.

The system must also be sufficiently secure to also provide the ability to deny the interaction,
since this is a personal choice of the individual - to disclose any personal information or not, we
live in an imperfect world in which many threats depend on information received by criminals.

## 1.2. Scope

In general, aSocial should become the most widespread system for interaction and storage of social
information, breaking down barriers between people and simplifying communication and processing of
such data, including freeing society from excessive centralization.

## 1.3. Core features

In order to build a better social system, you need to think about the main features in which I can
include:

  - **Modularity** - you can add or remove unimportant system components (UI - native, web,
    console) or replace important ones with alternatives (databases, for example). This will allow
    the system to be deployed on servers or pocket devices while meeting the specified security and
    responsiveness requirements.

  - **History** - is not only the ability to create elements (events, agents, etc.) at the right
    times, but also to observe their changes over time. For example, changing your friend's last
    name is important information, including the opportunity to see how it was before the change.

      - End-to-end change history with the ability to rewind
      - Timeline is a convenient display of events and areas of life (lived / worked) for agents
      - Grouping and tagging changes

  - **Persons** - are basically profiles of people or organizations with whom we interact in
    society. They may not have a profile, but exist as your (or collective) memory of them. You can
    leave your impressions about any person.

      - A profile can have several personas for different contexts, for example "Friends", "Work",
        "Relationships", and others. A separate address is generated for each of these, which are
        used as starting points for communication.
      - Each profile has a pair of keys, which are used to generate invitation keys to form a tree
        of connections, allowing you to work with information in terms of leaves, branches, or the
        whole tree.

  - **Events** - "what happens" and usually has a beginning and sometimes an end, like current
    place of residence, work, school, party, or when you find something important.

  - **Messages** - both communicating with other agents and requesting profile updates (which occur
    mainly in the background).

      - Tied to monetization to solve the spam problem.
      - Thread tree with the ability to transfer messages
      - Title / description / tags of branches
      - Easy import from other messengers / social networks
      - Update request (news from the media, for example)

  - **Overlays** - provide an opportunity to have your own perspective on any information you have
    received. They can be protected (password-protected) or unprotected, optional (enabled and
    disabled as needed), and associated with certain base objects or used for creating new objects.
    For example, you can add a friend to your network, even if they do not have a profile, and
    later, if they create one, link their profile. Or, in the overlay, you can change the person's
    name to an alternative one.

  - **Synchronization** - you own several devices and these devices can sync profiles between each
    other and perform different roles. For example, you can transfer your favorite collection of
    movies from the desktop, but still not have them on your phone.

  - **Sharing** - needed for transferring multimedia and other files. Uses bittorrent for upload
    and download.

## 1.4. Additional features

Based on the basic capabilities, additional functionality will be built (or even follow):

  - **Tree of life** - each of us has a generational history that we want to pass on to our
    children, so why not on the same social network? This is a convenient display of family ties
    between persons.
  - **Wills** - are a type of stored messages and the ability to set conditions for activating
    their sending or destruction. Free on controlled devices and paid on relays. For example, being
    offline for a month triggers the distribution of these messages.
  - **Contracts** - are messages in which you agree to certain obligations. These obligations are
    signed by both parties, they can block certain funds for a certain period, etc.
  - **Web interface** - is a module with which you can access your profile remotely using HTTP, you
    just need to launch the application, enable the module in the settings and open the desired
    host and port in the browser.
  - **Consensus** - is an addition to group chats to quickly resolve issues of agreement with a
    group of people. If the person agrees with the proposal, then one vote is added. If not, he
    opens a discussion to find a compromise among all participants. A decision is made as soon as
    each member of the group agrees.

# 2. Obstacles

## 2.1 General

The development of aSocial will face the following challenges:

- Most likely, it will be too difficult to do alone - I can miss important points and miscalculate
  in the main ideas. Therefore, peer review and discussion of concepts are necessary in the early
  stages of building the system. For this purpose, this document exists - to immerse potential
  collaborators in the problem and identify missed aspects in order to continue interaction in the
  future and jointly bring the system to a ready state.

The adoption of the new social network by users will face obvious challenges:

- **User Suspicion**: Anything new naturally raises questions, especially with so many innovations.
  The openness of the project can address concerns about suspicion - but additional efforts will
  likely be needed to demonstrate and explain the benefits. To bootstrap adoption, we can integrate
  with Bitcoin wallets for easy onboarding and target initial users in privacy-focused communities
  (e.g., geeks, programmers, security professionals).

- **Resistance from Politicians and Corporations**: They will try to obstruct progress because of
  conservatism and the complexity of managing a decentralized system. Also, lobbying by large
  corporations (such as Facebook) may impact overall development. The path of Bitcoin in this
  regard seems reasonable - users will choose a more convenient system regardless. Emerging
  regulations (e.g., EU DMA, US crypto laws) could impose KYC for monetization; aSocial's P2P
  design mitigates this but requires monitoring.

- **Counteraction from Intelligence Services**: They always want to access user communications
  under the guise of fighting terrorism, which is clearly misguided. They will try to embed
  backdoors under the guise of useful functions, but the open nature of the platform, community
  auditing, and the lack of centralized management should minimize this risk significantly.

- **AI-Generated Threats**: In 2026, AI spam, deepfakes, and automated attacks are rising;
  aSocial's payment thresholds and proof-of-humanity options (e.g., via Lightning challenges) will
  help, but we must integrate AI detection in overlays.

- **Ethical Misuse**: Decentralization could enable illicit networks; safeguards like
  user-controlled filters and wills/contracts include ethical defaults, but we emphasize personal
  responsibility.

## 2.2 Technical

Such distributed systems require special care in the development of the interaction protocol, since
it will be quite problematic to change it. Additional hurdles include scalability for large syncs
(addressed via opportunistic P2P), battery efficiency on mobile (via device role assignments), and
handling quantum threats (as a future priority).

## 2.3 Adoption and Metrics

To measure success, target an MVP with 1,000 active users in Year 1, focusing on sync speed (>90%
under 10s) and security audits. Phases: POC (core P2P messaging with payments), Beta (full features
like history/overlays), Release (mobile optimizations).

# 3. Industry and Market Risks

Social networks and messengers (these areas are the focus of this development), so entry to the
market is initially based on stakeholders (geeks, programmers, security professionals), from where
it may well spread further if convenient means of interaction are available.

In 2026, the market includes mature decentralized players (e.g., Nostr, Bluesky), but aSocial
differentiates with integrated spam-fighting monetization and deep privacy layers. Risks include
AI-driven competition; mitigation via open-source grants and community governance.

# 4. Budgetary Risks

Development is driven by pure enthusiasm, so budgetary risks are irrelevant, at least during the
POC development stage. As soon as the budget appears, they will need to be revised.

Development is driven by pure enthusiasm, so budgetary risks are irrelevant, at least during the
POC development stage. As the project scales, we can pursue open-source grants, Lightning bounties,
or crowdfunding via platforms like Gitcoin to fund audits and mobile ports. Once a budget appears,
risks like mismanagement will be revised with transparent community oversight.

# 5. Hardware

Target platforms for application launch:

  - Android - most phones and wearable devices in the world
  - Linux - open source and all servers on the Internet
  - Windows still dominates workstations and desktops
  - MacOS - working systems with UI, gradually superseding Windows
  - iPhone - Possibly, but most likely last due to the complexity of the ecosystem.

Separate hardware is not required, but the battery and communication capabilities between devices
will need to be considered.

# 6. Software

## 6.1. Architecture

The main platform uses Qt 6 with platform-specific add-ons. The choice of Qt 6, LevelDB, and
SQLCipher emphasizes trusted, battle-tested technologies that ensure the system remains
self-contained, multi-platform (supporting Android, Linux, Windows, macOS, and potentially iOS),
and performant on diverse hardware. While modern frameworks like Flutter or Tauri offer
alternatives, aSocial prioritizes reliability and developer familiarity to avoid dependencies that
could compromise security or introduce bloat. This stack allows seamless deployment on servers,
desktops, or pocket devices without sacrificing responsiveness or security.

The application is divided into two modules - backend and frontend:

### Backend

It deals with background activity - interacts with devices nearby and via the Internet, transmits
and receives data from other devices (for example, someone else's encrypted messages for further
transmission), receives updates. Stores intermediate already encrypted data in LevelDB.

### Frontend

User interface for managing profiles. Main storage is SQLCipher (SQLite with encryption).

## 6.2. Storage

For storage, a pair of local databases is used - the built-in unencrypted nosql storage LevelDB,
which is necessary to maintain the data transfer service, and the encrypted sql storage SqlCipher
(add-on over sqlite3) for storing profile data.

### LevelDB

It allows you to store key-value data that is not valuable for a potential attack - these are
connection lists, encrypted relay data... In general, the data that are necessary for interaction
between systems in hibernation mode. Also stores password-encrypted keys from available profiles.

### SqlCipher

An encrypted database for storing important profile information. It is decrypted in chunks as
needed using a long key.

### File storage

The described databases are stored on the local file system.

Regular files and media are stored unencrypted on the file system. Individual photos / videos can
be encrypted directly at the file level if necessary and will be available exclusively from aSocial,
since all known operating systems do not support decryption mechanisms at the file level.

## 6.3. Communication

The profile is a set of data encrypted with a cryptographic key, encrypted with a password.

When initializing a connection with another profile (social agent), a unique key for communication
is created - i.e. any communication with the outside world is carried out on a unique pair of keys
to prevent problems of reusing the same key. This key can only be used by those who received it.
You can also connect additional keys to the profile (or generate them) - they can be used to
initialize new connections, distribute your profile or receive donations / single messages.

Finding other people necessarily involves publishing information, but not necessarily the same
information posted on your main profile. You can create a separate layer containing information of
your choice to help others find you.

During communication, any available means are used - Internet, mesh networks, WiFi Direct,
Bluetooth, NFC, USB dongles, etc. As an interaction protocol, available open-source messenger
libraries should be examined. They already support end-to-end encryption, chats, NAT traversal, and
can be useful initially.

### 6.3.1 Interoperability

aSocial is designed to stand alone without relying on external systems for core functionality,
ensuring full user sovereignty and resilience. However, to enhance flexibility, it can integrate
with outside protocols as optional transports for communication (e.g., leveraging existing mesh
networks or APIs for NAT traversal) or for data fetching (e.g., importing profiles or events from
legacy social networks via open standards). This approach allows seamless onboarding from
centralized platforms while maintaining aSocial's decentralized ethos. Future extensions could
explore bridges to protocols like ActivityPub for read-only federation, but only as user-opt-in
features to avoid dependency risks.

## 6.4. Security

Security is the starting point of this project. It is not only about encryption, but also the
confidence in complete control over the information you own. Any permissions to provide information
to the outside are completely controlled by the user using filters and privacy levels (implemented
as overlays / layers), which are switched using different passwords and encrypted with different
keys.

I think everyone wants to appear a lot better for other people than they are - and we see that
social media profiles are mostly pretentious perfect. But modern social networks mostly forget that
there are those very real people behind a beautiful picture who are unable to store their data
somewhere other than their own heads. aSocial will improve this system and allow you to store and
share with certain people even confidential and secret information.

Thus, aSocial allows you to create different public identities or different faces can be
implemented using different profiles that can coexist on the same device. The base level of the
profile is your public identity.

When creating a profile for overlays, a randomly sized encrypted space is initialized. This space
is used for placing more secret information on an onion principle (each subsequent level can be
opened from the previous one with a unique password). Thus, any user can deny the very existence of
additional levels.

Initially, the platform supports full traffic encryption, end-to-end encryption of messages, and
the possibility of plausible denial of any action or the presence of a profile at all. Built-in
capabilities allow choosing an encryption system, depending on the secrecy of the message:

  - **Normal** - durable and compact encryption. Different algorithms can be selected. Message size
    equals the compressed original.
  - **Ambiguous** - enables sending two different messages in a single encrypted payload. The
    recipient can select which message to read depending on the specified decryption key. Encrypted
    messages are shuffled and the selection algorithm depends on key data.
  - **Shuffled with random message** - a variant of ambiguous encryption designed to assist other
    users with ambiguous encryption. The second key remains undisclosed, and random data generates
    the second message. Size: ~2x compressed original message. This is the default setting to
    enhance overall system security.
  - **Probabilistic** - provides maximum security through very large message sizes, suitable for
    advanced users who understand the security implications.

(This includes ambiguous encryption with noise, or probabilistic encryption, which increases
message cost due to larger size.)

To future-proof against emerging threats, aSocial will prioritize post-quantum cryptography (e.g.,
algorithms like Kyber) as a non-immediate upgrade. Initial releases focus on proven symmetric and
asymmetric encryption for broad compatibility, with quantum-resistant options planned for v2.0+ to
address potential quantum computing risks without overcomplicating the core system.

## 6.5. Spam, Ad & Monetization

The need to include the payment system in the social network is dictated by modern realities.
Firstly, it allows you to fight spam and advertising, and secondly, it is an opportunity to create
a new platform for the monetization of information and its distribution.

The main problem of spam and advertising is distraction, they literally gobble up our time, thus we
pay our most expensive resource for someone else's and mostly useless for most product promotion.
Remember incoming SMS on GSM networks - they are distracting, advertising on the Internet is
insanely distracting. Based on this, we must fight against such harmful phenomena, and the best way
to fight is to increase the payment for displaying the message. Thus, the user has the opportunity
to restrict the circle of persons, while not completely blocking messages from the outside.

In aSocial, you can set thresholds for receiving messages at several levels depending on the
frequency of communication. Thus, you will receive a fee and then the message appears; if the fee
is below the minimum threshold, it is ignored. The importance of the message is selected depending
on the level of payment:

  - Minimal - the message appears in the message list, there is no user notification. Used for
    subsequent messages in an active conversation.
  - Medium - additional to the minimum - normal pop-up window with a notification about the arrival
    of a message. It is used by default for the first message of a new conversation or for
    continuing a conversation after a long time.
  - Promoted - important messages with a special pop-up window and continuous notification that
    does not stop until the message is read.

Billing depends on the size of the message - a larger message will take longer to read.

A request (update) of information may also require payment - for example, a public figure or the
media may demand a payment for receiving public information about him / the latest news.

The technology used is Bitcoin (like the Lightning network). When you add a new contact, you set up
a new Lightning channel for exchanging funds - and they are sent back and forth when you exchange
messages. First, you receive a message and a transaction ID sent to the Bitcoin network to initiate
a lightning microtransaction channel. You receive it (if the funds sent are sufficient) and
continue communicating via the open lightning channel or close it and collect the funds received.
In fact, messages are sent along with the contract for the transfer of payment, so before
distracting you - aSocial will check the validity of the transaction.

When using relays (sending and temporary storage of a message - for example, due to the
inaccessibility of direct communication with the recipient), you additionally pay for the reception
and storage before sending it to the relay operator. In mesh networks (based on bluetooth / wifi
and other short-range communication systems), users can also charge a small fee for sending /
storing a message - but by default such a relationship is free, but it is regulated by the cache
size, the budget for application activity and the device charge (to regulate battery consumption
and other resources).

For example, you are a world famous creator of the Linux kernel, your time is insanely expensive -
and you have the right to set a fee for incoming messages: for close friends with whom you often
communicate - the fee may be close to zero, for outside unknowns - the fee is set depending on your
requirements.

A good cost guideline is your hourly salary (representing the value of one hour of distraction)
multiplied by 2-4 (since it is personal time). With an hourly rate of $20, the cost of a message
from a stranger would be priced at ~$40-$60 (~1mBTC with 1BTC = $34000), which seems quite
reasonable. One would certainly read a spam message for $60.

# 7. Predecessors

Multiple social networks and messengers have already been created in the world with varying degrees
of security, decentralization, convenience - so you need to understand on what foundation this
development begins. Here are a few major development milestones:

## 7.1. Centralized

### IRC / ICQ / Jabber

It all started with IM and chats, as people wanted to communicate. They allow you to exchange
messages, links, participate in discussions and store history, but they are very limited in
transferring your Self to the virtual space, i.e. not suitable for storing social information. They
have their own user database and are mostly centralized or federated.

### Facebook / Vk

The first popular social networks in the full sense of the word. They stood out for their ease of
use and the absence of a separate client (WEB-interface). Allowed to more or less conveniently
search for classmates and friends (allowed to create a profile with a good life description), as
well as share content with them in the form of a wall on your personal page - and this has gained
immense popularity among users.

Their main problem was centralization - this is a huge pot of honey (like any more or less large
database), attracting insiders and hackers. They also do not support p2p encryption, are saturated
with ads and sell / distribute your information to the right and left. I think today it is already
obvious that trusting your information for free is not only useless, but also harmful. The benefit
to the end user is negligible, and the damage received (from ads, leaks, errors, blocking,
censorship) is much greater.

### Twitter / Instagram / Youtube

Highly specialized, centralized social networks that have taken over all the shortcomings of their
older brothers.

### Skype / WhatsApp / Telegram / Signal / Wire / Slack

Active messengers that do their job pretty well, which are mainly aimed not at some semblance of
privacy, although they are still centralized in nature and focused on "not storing" classified
information (secret chats), which generally negates their usefulness.

## 7.2. Distributed

Unfortunately, [the majority of distributed social networks](https://en.wikipedia.org/wiki/Comparison_of_software_and_protocols_for_distributed_social_networking)
are somehow written in web-based languages, which imply client-server interaction and undermine the
very essence of distribution. These are usually complex to install, requiring databases and
non-trivial skills to maintain. Others are heavily outdated, investor-controlled or in their
infancy...

### RetroShare

Good, but drawn-out development of a decentralized messenger. Interesting developments in
decentralized communication using DHT, tor / i2p and NAT-piercing, which can be adapted to work in
aSocial. Disadvantages as a messenger include:

  - Poor support for mobile devices - In today's world, it is difficult to imagine an application
    that is not natively focused on wearable devices.
  - Outdated user interface design from 2000
  - It looks more like an attempt to combine disparate technologies for user convenience, rather
    than an attempt to rethink communication.

### Diaspora / Matrix (and other server-based)

It is aimed at combating centralization rather than user security and privacy. The concept of pods
implies a half-measure - your information is still owned by someone other than you. Of course, you
can raise your pod - but this is a very non-trivial task.

### OpenBazaar

Those who got close enough to decentralization and anonymity, but deviated from their original
ideas, the project of an open market and replacement for ebay / amazon marketplaces. He
successfully uses Bitcoin for settlements, but caved in under pressure from investors and turned
off the built-in anonymity functionality.

## 7.3 Post-2020 Developments

Since the early 2010s, decentralized social networks have evolved rapidly, incorporating Web3
elements like blockchain and crypto monetization. Below, we review key post-2020 projects, their
benefits, disadvantages, and how aSocial improves upon them. This positions aSocial as an
evolution: true P2P with user-controlled monetization, deep history tracking, and multi-layer
privacy - without heavy blockchain reliance or server dependencies.

- **Nostr (Notes and Other Stuff Transmitted by Relays)**: A relay-based protocol for
  censorship-resistant posting, with Lightning "zaps" for micropayments.
  - **Benefits**: Highly decentralized (no central servers), privacy-focused, resilient to
    censorship, open-source for custom clients, integrates Bitcoin for value transfer.
  - **Disadvantages**: Stalled growth in 2025 (activity flatlined), poor mobile support, outdated
    UIs, requires technical knowledge, vulnerable to spam/obscene content without strong filters.
  - **Comparison to aSocial**: Like Nostr, aSocial uses P2P for resilience and Lightning for
    monetization to fight spam. However, aSocial adds advanced history rewinds, overlays for
    contextual privacy, and better multi-platform UIs - addressing Nostr's adoption barriers.

- **Mastodon/Fediverse**: Federated servers using ActivityPub for microblogging and interconnected
  apps.
  - **Benefits**: Decentralized with community governance, privacy controls, no ads, chronological
    feeds, interoperable with other Fediverse tools (e.g., Pixelfed for images).
  - **Disadvantages**: Fragmented experiences across instances, technical setup barriers, smaller
    user base (~10M in 2026), federation complexities leading to inconsistencies.
  - **Comparison to aSocial**: aSocial avoids server reliance with true P2P, offering deeper user
    data control and spam-resistant payments - surpassing Mastodon's half-measures on
    centralization.

- **Bluesky/AT Protocol**: User-owned data with portable identities and customizable feeds.
  - **Benefits**: Transparency via open standards, user empowerment (own your data/followers),
    growing ecosystem of apps, resilient against platform lock-in.
  - **Disadvantages**: Still semi-centralized (Bluesky controls much infrastructure in 2026),
    unclear monetization, smaller scale (~36M users), technical growing pains.
  - **Comparison to aSocial**: Both emphasize portability, but aSocial's encryption layers and P2P
    sync provide stronger privacy without company oversight, plus built-in monetization.

- **Farcaster**: Hybrid on/off-chain protocol with Ethereum integration and "Frames" for in-app
  dApps.
  - **Benefits**: Censorship-resistant, user-owned data, resilient network, monetization via
    fees/Frames, interoperable with Web3 (e.g., wallets).
  - **Disadvantages**: Declining revenue (~$10K/month in late 2025), high scalability costs, low
    adoption (~100K users), fees deter casual use.
  - **Comparison to aSocial**: aSocial shares hybrid efficiency but uses Bitcoin/Lightning for
    accessible payments without Ethereum's gas fees, focusing on history and overlays for richer
    interactions.

- **Lens Protocol**: NFT-based social graph on Polygon for decentralized profiles and monetization.
  - **Benefits**: True ownership via NFT profiles, interoperability across Web3, monetization
    (subscriptions/NFTs), censorship-resistant, no algorithmic manipulation.
  - **Disadvantages**: High entry barriers (wallets/NFT minting), on-chain costs/performance
    issues, immature content moderation, early-stage adoption.
  - **Comparison to aSocial**: aSocial uses key-based identities without blockchain bloat,
    integrating payments for spam control and adding generational features like Tree of Life.

- **Patreon (and similar centralized monetization platforms like Substack)**: Crowdfunding for
  creators with recurring support.
  - **Benefits**: Easy setup for recurring income, community perks, supports diverse content,
    builds loyalty.
  - **Disadvantages**: High fees (8-12% + processing), no data ownership, platform
    rules/censorship, tax complexities, reliant on centralized infrastructure.
  - **Comparison to aSocial**: aSocial embeds decentralized monetization (Lightning channels) to
    reward creators without cuts or control, fighting spam while enabling direct, private
    support - far superior for freedom-focused users.

# Conclusion

Overall, the aSocial project should be a significant benefit for everyone. I think it's needed.

If you're excited about building a truly free social future, contribute via [GitHub](https://github.com/state-of-the-art/asocial),
discuss on forums, or reach out for collaboration. Together, we can make aSocial a
reality - empowering users worldwide.
