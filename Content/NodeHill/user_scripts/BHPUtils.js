// config
const prefix = "!"
const killbrickName = "Killbrick"

// get all killbricks
const killBricks = Game.world.bricks.filter(brick => brick.name === killbrickName);

// chat utils
Game.on("playerJoin", (player) => {
   player.on("chatted", (message) => {

        // reset command
        if (message == prefix + "reset") {
            player.kill()
        }
        
        // help command
        if (message == prefix + "help") {
            setTimeout(() => {
                player.message("--== Commands ==--")
                player.message("!help: Sends this message")
                player.message("!reset: Resets your character")
            }, 500)
        }
   })

   // join message
   player.message("Use !help to see commands")
   // global join message
   Game.messageAll("[#FFC300]" + player.username + " Joined the game!")
})

// global leave message
Game.on("playerLeave", (player) => {
    Game.messageAll("[#FFC300]" + player.username + " Left the game!")
})

// body colors
Game.on('customBodyColors', (player, colors) => {
    player.on('avatarLoaded', () => {
        const outfit = new Outfit();
        outfit.head(colors.headColor);
        outfit.torso(colors.torsoColor);
        outfit.leftArm(colors.leftArmColor);
        outfit.rightArm(colors.rightArmColor);
        outfit.leftLeg(colors.leftLegColor);
        outfit.rightLeg(colors.rightLegColor);
        player.setOutfit(outfit);
    });
});

// killbrick functionality
killBricks.forEach(brick => {
    brick.touching((player) => {
        player.kill();
    });
});