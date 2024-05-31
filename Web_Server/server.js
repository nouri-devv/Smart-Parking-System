const express = require('express');
const bodyParser = require('body-parser');
const nodemailer = require('nodemailer');
const path = require('path');
const fs = require('fs');
const { createExpiringObject, getObject, markCodeAsUsed, checkDataIntegrity, validateData } = require('./utils/expiringObjects');

const app = express();
const port = 5000;

// Set EJS as the templating engine
app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));

// Middleware to parse URL-encoded form data and JSON bodies
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());
// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

app.get('/', (req, res) => {
    fs.readFile('spots.json', 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error reading file');
            return;
        }
        const spots = JSON.parse(data);
        const spacesAvailable = spots.filter(spot => !spot.occupied).length;
        res.render('index', { spacesAvailable, spots });
    });
});

app.post('/send-email', (req, res) => {
    const { email, fname, lname, plate, spot } = req.body;
    const code = Math.floor(100000 + Math.random() * 900000); // Generate a random 6-digit code

    const transporter = nodemailer.createTransport({
        service: 'gmail',
        auth: {
            user: 'nouri.mst0602@gmail.com',
            pass: 'pkmm mczb ujgh skrh'
        }
    });

    const mailOptions = {
        from: 'nouri.mst0602@gmail.com',
        to: email,
        subject: 'SIT210 Smart Parking System Verification Code',
        text: `Thank you for using the SIT210 Smart Parking System.\n\nYour verification code is ${code}.\nYou have 1 hour to use this code!`
    };

    transporter.sendMail(mailOptions, (error, info) => {
        if (error) {
            console.log(error);
            res.status(500).send('Error sending email');
        } else {
            console.log('Email sent: ' + info.response);
            createExpiringObject(email, code, 3600000, spot); // Pass spot number to createExpiringObject with 1-hour expiry
            markSpotAsTaken(spot, () => {
                res.render('verify'); // Render the verification page
            });
        }
    });
});

const markSpotAsTaken = (spotNumber, callback) => {
    fs.readFile('spots.json', 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            return;
        }

        let spots = JSON.parse(data);
        const spot = spots.find(spot => spot['spot number'] == spotNumber);

        if (spot) {
            spot.occupied = true;

            fs.writeFile('spots.json', JSON.stringify(spots, null, 2), (err) => {
                if (err) {
                    console.error(err);
                    return;
                }
                callback();
            });
        }
    });
};

app.get('/available-spots', (req, res) => {
    fs.readFile('spots.json', 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error reading file');
            return;
        }
        const spots = JSON.parse(data);
        const spacesAvailable = spots.filter(spot => !spot.occupied).length;
        res.json({ spacesAvailable, spots });
    });
});

app.post('/spot-free', (req, res) => {
    const { spotNumber, occupied } = req.body; // Get spotNumber and occupied from request body
  
    // Read the spots.json file
    fs.readFile('spots.json', 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error reading file');
            return;
        }

        let spots = JSON.parse(data);
        const spot = spots.find(spot => spot["spot number"] == spotNumber);

        if (spot) {
            spot.occupied = occupied; // Set the occupied status based on the received value
        
            // Write the updated spots data back to spots.json
            fs.writeFile('spots.json', JSON.stringify(spots, null, 2), (err) => {
                if (err) {
                    console.error(err);
                    res.status(500).send('Error updating file');
                    return;
                }
                res.send('Spot updated successfully');
            });
        } else {
            res.status(404).send('Spot not found');
        }
    });
});

// Verify code endpoint
app.post('/verify-code', (req, res) => {
    const { email, code } = req.body; // Extract email and code from request body

    console.log(`Received verification attempt: email=${email}, code=${code}`); // Debug logging

    const storedCodeObject = getObject(email); // Retrieve the stored code using the email

    if (storedCodeObject && storedCodeObject.code == code) {
        markCodeAsUsed(email); // Mark code as used
        res.send("Gates are opening");
    } else {
        res.send("Incorrect code. Please try again.");
    }
});

// Start the server on the specified port
app.listen(port, () => {
    console.log(`Server is running at http://192.168.137.52:${port}`);
});