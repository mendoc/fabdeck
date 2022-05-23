require('dotenv').config();
const axios = require('axios')
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

        if (err === false) {
            // On récupère l'action de l'event
            const action = await getAction(evt)

            if (action) {
                console.log(action)
                switch (action.type) {
                    case "tlg":
                        sendTelegramMessage(action.params.tlg, action.params.chatid)
                        break;
                }
            }
        }
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

async function updateEvent(evt) {
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

async function getAction(evt) {
    try {
        const snap = await db.collection("actions").doc(evt).get();
        return snap.data()
    } catch (err) {
        console.log(err)
        return true
    }
}

async function sendTelegramMessage(msg, chatID) {
    const bot_token = process.env.BOT_TOKEN
    msg = msg.normalize('NFD').replace(/[\u0300-\u036f]/g, '');

    const url = 'https://api.telegram.org/bot' + bot_token + '/sendMessage?chat_id=' + chatID + '&parse_mode=Markdown&text=' + msg
    const res = await axios.get(url).catch(console.log);
    console.log(res.data.ok)
}