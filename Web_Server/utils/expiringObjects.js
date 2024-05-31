const fs = require('fs');
const nodemailer = require('nodemailer');

// Load existing objects from the file 'objects.txt'
let expiringObjects = {};
if (fs.existsSync('objects.txt')) {
  const data = fs.readFileSync('objects.txt', 'utf8');
  expiringObjects = JSON.parse(data);
}

// Save the current state of objects to 'objects.txt'
const saveObjects = () => {
  fs.writeFileSync('objects.txt', JSON.stringify(expiringObjects));
};

// Create an expiring object with a specified duration
const createExpiringObject = (email, code, duration, spotNumber) => {
  const expireTime = Date.now() + duration;
  expiringObjects[email] = { code, expireTime, used: false, spotNumber };
  saveObjects();

  // Set a timeout to check for expiration
  setTimeout(() => {
    if (expiringObjects[email] && !expiringObjects[email].used) {
      sendExpirationEmail(email); // Send expiration email
      markSpotAsFree(expiringObjects[email].spotNumber); // Mark the spot as free
      delete expiringObjects[email]; // Delete the expired object
      saveObjects(); // Save the current state of objects
    }
  }, duration);
};

// Retrieve the code associated with an email if it has not expired
const getObject = (email) => {
  const obj = expiringObjects[email];
  if (obj && obj.expireTime > Date.now()) {
    return obj.code;
  } else {
    return null;
  }
};

// Mark an object as used and delete it
const deleteObject = (email) => {
  if (expiringObjects[email]) {
    expiringObjects[email].used = true;
    saveObjects();
    delete expiringObjects[email];
    saveObjects();
  }
};

// Send an expiration email to notify the user
const sendExpirationEmail = (email) => {
  var transporter = nodemailer.createTransport({
    service: 'gmail',
    auth: {
      user: 'nouri.mst0602@gmail.com',
      pass: 'pkmm mczb ujgh skrh'
    }
  });

  var mailOptions = {
    from: 'nouri.mst0602@gmail.com',
    to: email,
    subject: 'SIT210 Smart Parking System Verification Code Expired',
    text: 'Your verification code has expired for the SIT210 Smart Parking System. Please request a new one.'
  };

  // Send the expiration email
  transporter.sendMail(mailOptions, function(error, info) {
    if (error) {
      console.log(error);
    } else {
      console.log('Expiration email sent: ' + info.response);
    }
  });
};

// Mark a spot as free in spots.json
const markSpotAsFree = (spotNumber) => {
  fs.readFile('spots.json', 'utf8', (err, data) => {
    if (err) {
      console.error('Error reading file', err);
      return;
    }

    let spots = JSON.parse(data);
    const spot = spots.find(spot => spot['spot number'] == spotNumber);

    if (spot) {
      spot.occupied = false;

      fs.writeFile('spots.json', JSON.stringify(spots, null, 2), (err) => {
        if (err) {
          console.error('Error updating file', err);
        } else {
          console.log(`Spot ${spotNumber} marked as free`);
        }
      });
    }
  });
};

// Data integrity check function
const checkDataIntegrity = (data) => {
  const checksum = data.split('').reduce((acc, char) => acc + char.charCodeAt(0), 0);
  return checksum;
};

// Validate data with checksum
const validateData = (data, expectedChecksum) => {
  return checkDataIntegrity(data) === expectedChecksum;
};

module.exports = { createExpiringObject, getObject, deleteObject, checkDataIntegrity, validateData };