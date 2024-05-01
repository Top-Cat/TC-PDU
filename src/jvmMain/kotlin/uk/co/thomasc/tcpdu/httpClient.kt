package uk.co.thomasc.tcpdu

import io.ktor.client.HttpClient
import io.ktor.client.HttpClientConfig
import io.ktor.client.engine.apache.Apache
import io.ktor.client.engine.apache.ApacheEngineConfig
import io.ktor.client.plugins.HttpTimeout

private fun setupClient(block: HttpClientConfig<ApacheEngineConfig>.() -> Unit = {}) = HttpClient(Apache) {
    install(HttpTimeout)

    engine {
        customizeClient {
            setMaxConnTotal(100)
            setMaxConnPerRoute(20)
        }
    }

    block()
}

val client = setupClient()
