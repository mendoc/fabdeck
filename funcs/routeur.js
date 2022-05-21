require('dotenv').config();

const admin = require('firebase-admin');

const serviceAccount = JSON.parse(process.env.SERVICE_ACCOUNT);

admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: process.env.DATABASE_URL
});


const db = admin.firestore();

exports.handler = async (e, context) => {
    let evt = e.queryStringParameters.e

    if (evt) {
        const err = await updateEvent(evt)
        return {
            statusCode: 200,
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ erreur: err })
        }
    } else {
        return {
            statusCode: 401,
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ message: "Veuillez renseigner un evenement" })
        }
    }
}

async function updateEvent(evt, cb) {
    const evtDoc = {
        targetAt: admin.firestore.FieldValue.serverTimestamp(),
    }

    try {
        await db.collection("events").doc(evt).set(evtDoc);
        return false
    } catch (err) {
        console.log(err)
        return true
    }
}