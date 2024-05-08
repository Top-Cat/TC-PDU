package uk.co.thomasc.tcpdu.util

import kotlinx.serialization.json.Json

val pduJsonUgly = Json {
    ignoreUnknownKeys = true
}

val pduJson = Json(pduJsonUgly) {
    prettyPrint = true
}
