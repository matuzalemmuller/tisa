### Usage

Install [Docker Compose](https://docs.docker.com/compose/install/).

Start Node-RED and flask servers:

```
docker-compose up
```

Stop Node-RED and flask servers:

```
docker-compose down
```

* Node-RED is available on port 80 as is set up to run over an insecure connection. To access the Node-RED server over HTTPS, uncomment the lines 141-144 and 165 in the [settings.js](node-red/settings.js) file and replace the port `80` by `443` in the `docker-compose.yml` file. It is also necessary to add the files `privkey.pem` and `fullchain.pem` to this directory so the certificate is applied to the server.
* Node-RED password is `admin1234`.
* To replace the address of the Firebase database used by the program, change the `firebaseurl` value in the line 55 of [flows.json](node-red/settings.js). Additional configuration is also required for the `mosquitto` nodes.

*Note: The [node-red-contrib-firebase](https://github.com/deldrid1/node-red-contrib-firebase) lib has been discontinued by the developer and some bugs can happen. If any issues arise, just try to re-add the database credentials through the Node-RED interface and check if the programs reconnects to the database and resumes activity.*

----

The file `startup-script.txt` contains all the commands to install `docker` and `docker-compose` in a fresh Debian buster installation.