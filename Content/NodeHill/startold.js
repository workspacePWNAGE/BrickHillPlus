const nh = require('node-hill');
nh.startServer({
    hostKey: '',
    gameId: 478,
    port: 42480,
    ip: "127.0.0.1",
    local: true,
    mapDirectory: './maps/',
    map: 'Castle.brk',
    scripts: './user_scripts',
    modules: ["fs", "path", "http"]
});
