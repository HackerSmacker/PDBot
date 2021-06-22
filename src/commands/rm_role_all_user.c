#include <orca/discord.h>
#include "../libs/bot_include.h"

void rm_role_all_user(struct discord *client, const struct discord_user *bot, const struct discord_message *msg) {
    if (msg->author->bot) return;

    uint8_t has_role = 0;
    char author_avatar_url[AVATAR_URL_LEN], owner_role_mention[ROLE_MENTION_LEN];
    struct discord_guild_member *guild_member = discord_guild_member_alloc();
    struct discord_guild *guild = discord_guild_alloc();
    struct discord_embed *embed = discord_embed_alloc();
    struct discord_create_message_params params = {.embed = embed};

    embed->timestamp = msg->timestamp;
    role_mention(owner_role_mention, R_OWNER);
    get_avatar_url(author_avatar_url, msg->author);
    snprintf(embed->footer->text, 2049, "Author ID: %lu", msg->author->id);
    discord_embed_set_author(embed, msg->author->username, NULL, author_avatar_url, NULL);
    discord_get_guild_member(client, msg->guild_id, msg->author->id, guild_member);

    for (size_t i=0; guild_member->roles[i]; i++) {
        if (guild_member->roles[i]->value == R_OWNER) {
            has_role = 1;
            break;
        }
    }

    if (!has_role) {
        embed->color = COLOR_RED;
        snprintf(embed->title, 257, "No permission!");
        discord_embed_add_field(embed, "Required role", owner_role_mention, true);

        discord_create_message(client, msg->channel_id, &params, NULL);

        discord_guild_member_free(guild_member);
        discord_guild_free(guild);
        discord_embed_free(embed);

        return;
    }
    else {
        u64_snowflake_t arg_role_id[MAX_GUILD_ROLES];
        regex_t regex;
        char *tok = strtok(msg->content, " ");

        memset(&arg_role_id, 0, MAX_GUILD_ROLES);

        if (!tok) {
            embed->color = COLOR_RED;
            snprintf(embed->title, 257, "No role_id provided!");
            snprintf(embed->description, 2049, "Error arg[1]: NULL");

            discord_create_message(client, msg->channel_id, &params, NULL);

            return;
        }

        get_args_ids_as_int(arg_role_id, msg, " ");
        // for (size_t i=0; tok; i++) {
        //     puts(tok);
        //     if (!regcomp(&regex, REGEX_ID, 0)) {
        //         arg_role_id[i] = strtoul(tok, NULL, 10);

        //         tok = strtok(NULL, " ");
        //     }
        //     else {
        //         embed->color = COLOR_RED;
        //         snprintf(embed->title, 257, "Not an id!");
        //         snprintf(embed->description, 2049, "Error arg[%lu]: %s", i+1, tok);
        //         // snprintf(embed->title, 257, "Must specify at least 1 role_id!");
        //         // snprintf(embed->description, 2049, "Was not able to process command");

        //         discord_create_message(client, msg->channel_id, &params, NULL);

        //         return;
        //     }
        // }

        struct discord_message *embed_message = discord_message_alloc();
        struct discord_edit_message_params edit_params = {.embed = embed};
        struct discord_list_guild_members_params list_guild_members_params = {.limit = 1000};
        struct discord_guild_preview guild_preview;
        embed->color = COLOR_YELLOW;

        discord_get_guild(client, msg->guild_id, guild);
        discord_get_guild_preview(client, msg->guild_id, &guild_preview);
        discord_list_guild_members(client, msg->guild_id, &list_guild_members_params, &guild->members);

        snprintf(embed->title, 257, "Please wait...");
        snprintf(embed->description, 2049, "Removing roles from %d members", guild_preview.approximate_member_count);

        discord_create_message(client, msg->channel_id, &params, embed_message);

        for (size_t i=0; arg_role_id[i]; i++) {
            for (size_t j=0; guild->members[j]; j++) {
                if (!guild->members[j]->user->bot) {
                    discord_remove_guild_member_role(client, msg->guild_id, guild->members[j]->user->id, arg_role_id[i]);
                }
            }
        }

        embed->color = COLOR_PINK_PANTONE;
        snprintf(embed->title, 257, "Done! Took %lu ms", cee_timestamp_ms()-msg->timestamp);
        snprintf(embed->description, 2049, "Removed roles from %d members", guild_preview.approximate_member_count);

        discord_edit_message(client, msg->channel_id, embed_message->id, &edit_params, NULL);

        discord_guild_member_free(guild_member);
        discord_guild_free(guild);
        discord_embed_free(embed);
        discord_message_free(embed_message);

        return;
    }

    return;
}
