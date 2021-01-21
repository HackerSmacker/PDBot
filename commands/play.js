const { embed, display } = require("../utilities/display.js");
const { SearchQuery } = require('erela.js');

exports.name = 'play';
exports.type = 'Music';
exports.info = 'Plays Music [Either with search term or link]'
exports.usage = '<yt/youtube/sc/soundcloud/link> <search term/undefined>'
exports.alias = ['p'];
exports.root = false;
exports.admin = false;
exports.mod = false;

exports.run = async({client, message, args, args1=undefined}) => {
    let player = client.Music.player
    if (!player) {
        await player.create({
            guild: message.guild.id,
            voiceChannel: message.member.voice.channel.id,
            textChannel: message.channel.id,
            selfMute: false,
            selfDeafen: true,
        })
    }

    if (player.state != 'CONNECTED') {player.connect(); message.channel.send(`Joined \`${message.member.voice.channel.name}\` and binded to \`${message.channel.name}\``)}
    if (player.state == 'CONNECTED' && player.textChannel != message.channel.id && player.voiceChannel != message.member.voice.channel.id) {
        return message.reply(`You're not in the correct text channel/voice channel or both.`)
    }

    let res;
    let args = args.string();
    if (args.match(/(http(s)?:\/\/.)?(www\.)/)) {
        res = await player.search(args.string(), message.author.id)
    } else if (args == 'yt' || args == 'youtube') {
        const eres = new SearchQuery(args1, 'youtube')
        res = await player.search(eres, message.author.id)
    } else if (args  == 'sc' || args == 'soundcloud') {
        const eres = new SearchQuery(args1, 'soundcloud')
        res = await player.search(eres, message.author.id)
    }

    switch(res.loadType) {
        case 'SEARCH_RESULT':
        case 'TRACK_LOADED':
            player.queue.add(res.tracks[0])
            embed = embed('BLUE', `Added To Queue`, undefined, [
                {
                    name: undefined,
                    value: [
                        display({
                            name: 'Title',
                            usage: `[${res.tracks[0].title}](${res.tracks[0].uri})`
                        }),
                        display({
                            name: `Artist`,
                            usage: `${res.tracks[0].author}`
                        }),
                        display({
                            name: `Requester`,
                            usage: `${message.author.username}`
                        }),
                    ],
                },
            ])
            await message.channel.send(embed)
            // Prevents current playing track gets skipped by queued track
            if (!player.playing && !player.paused && !player.queue.size) await player.play();
            break;
        case 'PLAYLIST_LOADED':
            if (!res.playlist) return;
            embed = embed("BLUE", `Added To Queue`, undefined, [
                {
                    name: undefined,
                    value: [
                        display({
                            name: "Playlist Name",
                            value: `[${res.playlist.title}](${res.playlist.uri})`
                        }),
                        display({
                            name: "Total Tracks",
                            value: `${res.tracks.length}`
                        }),
                        display({
                            name: "Requester",
                            value: `${message.author.username}`
                        })
                    ]
                }
            ])
            message.channel.send(embed)
            if (!player.playing && !player.paused && !player.queue.size) await player.play();
            break;
        case 'NO_MATCHES':
            message.reply(`No matches found for ${res.query}`)
            break;
        case 'LOAD_FAILED':
            message.reply("Failed to load the tracks requested.")
            break;        
    }
}