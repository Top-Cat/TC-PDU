package uk.co.thomasc.tcpdu

import com.github.mustachejava.DefaultMustacheFactory
import io.ktor.http.HttpStatusCode
import io.ktor.server.application.Application
import io.ktor.server.application.install
import io.ktor.server.http.content.staticResources
import io.ktor.server.locations.Locations
import io.ktor.server.mustache.Mustache
import io.ktor.server.netty.EngineMain
import io.ktor.server.plugins.NotFoundException
import io.ktor.server.plugins.statuspages.StatusPages
import io.ktor.server.response.respond
import io.ktor.server.routing.routing

fun Application.setup() {
    install(Locations)
    install(StatusPages) {
        exception<NotFoundException> { call, _ ->
            call.respond(HttpStatusCode.NotFound, "Not Found")
        }

        exception<Throwable> { call, cause ->
            call.respond(HttpStatusCode.InternalServerError, "Internal Server Error")
            throw cause
        }
    }

    install(Mustache) {
        mustacheFactory = DefaultMustacheFactory("templates")
    }

    routing {
        with(MainRoute()) { setup() }
        staticResources("/static", "static")
    }
}

fun main(args: Array<String>) {
    EngineMain.main(args)
}
