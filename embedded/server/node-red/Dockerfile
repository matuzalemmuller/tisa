FROM nodered/node-red:1.2.1-minimal

# Copy package.json to the WORKDIR so npm builds all
# of your added nodes modules for Node-RED
COPY package.json .
RUN npm install --unsafe-perm --no-update-notifier --no-fund --only=production
#RUN npm install --unsafe-perm node-red-contrib-firebase@1.1.1 node-red-dashboard@2.23.4

# Copy _your_ Node-RED project files into place
# NOTE: This will only work if you DO NOT later mount /data as an external volume.
#       If you need to use an external volume for persistence then
#       copy your settings and flows files to that volume instead.
COPY settings.js /data/settings.js
#COPY flows_cred.json /data/flows_cred.json
COPY flows.json /data/flows.json