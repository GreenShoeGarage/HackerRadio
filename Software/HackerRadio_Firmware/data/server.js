var http = require('http');
var fs = require('fs');


http.createServer(function (req, res) {
    if (req.url === '/') {
        fs.readFile('index.html', function (err, data) {
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.write(data);
            res.end();
        });
    }
    else if (req.url === '/style.css') {
        fs.readFile('style.css', function (err, data) {
            res.writeHead(200, { "Content-Type": "text/css" });
            res.write(data);
            res.end();
        });
    }

    else if (req.url === '/images/gsg.jpg') {
        fs.readFile('images/gsg.jpg', function (err, content) {
            if (err) {
                res.writeHead(400, { 'Content-type': 'text/html' })
                console.log(err);
                res.end("No such image");
            } else {
                //specify the content type in the response will be an image
                res.writeHead(200, { 'Content-type': 'image/jpg' });
                res.end(content);
            }
        });
    }
}).listen(1337); 
console.log("Server running at http://localhost:1337");