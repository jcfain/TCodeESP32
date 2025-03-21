

In order to make changes to the web page files. The files must be minified somehow. 
You can use any minifyer you wish but please use the following for pull requests.

Install dependencies: nodejs and npm

In Linux Debian: 
sudo apt install nodejs npm
In Windows: 
Download nodejs and and npm and install them.

Run to install dependencies: 
npm install

### minify.js
Minify.js copies the base data in `dataEdit/www` to the `data/www` folder while minifying css, html, and javascript.
Make any edits in `dataEdit` 

Run from the ESP32 directory:
node minify.js 

Note: This may partially overwrite the data directory.
