#include <concord/discord.h>
#include <concord/cog-utils.h>
#include "../libs/bot_include.h"

void ping(struct discord *client, const struct discord_message *msg) {
    if (msg->author->bot) return;

    char content[TIMESTAMP_STR_LEN+10];

    snprintf(content, TIMESTAMP_STR_LEN+10, "Pong! %lums", cog_timestamp_ms()-msg->timestamp);
    struct discord_create_message_params params = {.content = content};

    discord_create_message(client, msg->channel_id, &params, NULL);
}
