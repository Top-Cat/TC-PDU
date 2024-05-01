package uk.co.thomasc.tcpdu

import io.ktor.client.request.get
import io.ktor.client.request.post
import io.ktor.client.request.setBody
import io.ktor.client.statement.bodyAsText
import io.ktor.server.application.ApplicationCall
import io.ktor.server.application.call
import io.ktor.server.locations.Location
import io.ktor.server.locations.get
import io.ktor.server.locations.post
import io.ktor.server.mustache.MustacheContent
import io.ktor.server.request.receiveStream
import io.ktor.server.response.respond
import io.ktor.server.routing.Route
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

const val apiRoot = "http://10.2.2.58"

@Location("/")
class MainRoute {
    @Location("/{path?}")
    data class Home(val path: String, val api: MainRoute)

    @Location("/")
    data class Root(val api: MainRoute)

    @Location("/proxy/{path?}")
    data class Proxy(val path: String, val api: MainRoute)

    @Location("/proxy/{path?}/{path2?}")
    data class Proxy2(val path: String, val path2: String, val api: MainRoute)

    fun Route.setup() {
        get<Root> {
            call.respond(
                MustacheContent("home.mustache", null)
            )
        }

        get<Home> {
            call.respond(
                MustacheContent("home.mustache", null)
            )
        }

        suspend fun getProxy(call: ApplicationCall, path: String) {
            call.respond(
                client.get("$apiRoot/$path") {
                    call.request.headers.forEach { key, strings ->
                        headers.appendAll(key, strings)
                    }
                }.bodyAsText()
            )
        }

        suspend fun postProxy(call: ApplicationCall, path: String) {
            val result = client.post("$apiRoot/$path") {
                call.request.headers.forEach { key, strings ->
                    headers.appendAll(key, strings)
                }
                val bytes = withContext(Dispatchers.IO) {
                    call.receiveStream().readAllBytes()
                }
                setBody(bytes)
            }

            result.headers.forEach { key, strings ->
                strings.forEach { v ->
                    call.response.headers.append(key, v)
                }
            }

            call.respond(result.status, result.bodyAsText())
        }

        get<Proxy> {
            getProxy(call, it.path)
        }

        get<Proxy2> {
            getProxy(call, "${it.path}/${it.path2}")
        }

        post<Proxy> {
            postProxy(call, it.path)
        }

        post<Proxy2> {
            postProxy(call, "${it.path}/${it.path2}")
        }
    }
}