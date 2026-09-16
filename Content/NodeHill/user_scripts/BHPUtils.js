// requires
const http = getModule('http');
const fs = getModule('fs');
const path = getModule('path');

// config
const prefix = "!"
const killbrickName = "Killbrick"
const assetDir = './assets';
const assetsPort = 42481;

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

// set up asset server
// http.createServer((req, res) => {
//     const id = decodeURIComponent(req.url.slice(1));
//     if (!/^[a-zA-Z0-9_-]{1,64}$/.test(id)) {
//         res.writeHead(400);
//         return res.end();
//     }
//     fs.readFile(path.join(assetDir, id + '.png'), (err, data) => {
//         if (err) {
//             res.writeHead(404);
//             return res.end();
//         }
//         res.writeHead(200, { 'Content-Type': 'image/png' });
//         res.end(data);
//     });
// }).listen(FACE_PORT, () => console.log(`asset server running on port ${assetsPort}`));