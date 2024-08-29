package uk.co.thomasc.tcpdu.page

import external.axiosGet
import kotlinx.datetime.Instant
import kotlinx.datetime.LocalDate
import kotlinx.datetime.LocalDateTime
import kotlinx.datetime.LocalTime
import kotlinx.datetime.TimeZone
import kotlinx.datetime.format
import kotlinx.datetime.format.char
import kotlinx.datetime.toLocalDateTime
import kotlinx.html.js.onClickFunction
import kotlinx.serialization.Serializable
import react.Props
import react.dom.a
import react.dom.li
import react.dom.nav
import react.dom.span
import react.dom.table
import react.dom.tbody
import react.dom.td
import react.dom.th
import react.dom.thead
import react.dom.tr
import react.dom.ul
import react.fc
import react.router.useNavigate
import react.useEffect
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.util.EnumAsLongSerializer
import kotlin.math.max
import kotlin.math.min

@Serializable
data class LogLine(val time: Long, val type: LogType, val user: String, val message: String) {
    fun formattedTime() = Instant
        .fromEpochMilliseconds(time)
        .toLocalDateTime(TimeZone.currentSystemDefault())
        .format(dtf)

    companion object {
        val dtf = LocalDateTime.Format {
            date(LocalDate.Formats.ISO)
            char(' ')
            time(LocalTime.Formats.ISO)
        }
    }
}

@Serializable(with = LogType.LogTypeSerializer::class)
enum class LogType(val enc: Long, val color: String, val human: String) {
    UNKNOWN(-1, "dark", "UNKNOWN"),
    OUTLET_STATE(0, "info", "Outlet State"),
    DEVICE_IP(1, "warning", "IP"),
    FIRMWARE(2, "primary", "Firmware"),
    CRASH(3, "danger", "Crash"),
    LOGIN_SUCCESS(4, "success", "Login"),
    LOGIN_FAILURE(5, "danger", "Login Failure"),
    CONFIG(6, "active", "Config"),
    OVERLOAD(7, "danger", "Overload"),
    TRIP(8, "danger", "Output Trip"),
    ALARM(9, "danger", "Output Alarm"),
    BOOT(10, "warning", "Boot Info");

    class LogTypeSerializer : EnumAsLongSerializer<LogType>(
        "LogType",
        { it.enc },
        { v -> LogType.entries.firstOrNull { it.enc == v } ?: UNKNOWN }
    )
}

@Serializable
data class LogPage(val logs: List<LogLine>, val count: Int, val page: Int, val pages: Int)

val logsPage = fc<Props> {
    val history = useNavigate()
    val (logs, setLogs) = useState(listOf<LogLine>())
    val (page, setPage) = useState(1)
    val (pages, setPages) = useState(1)

    useEffect(page) {
        axiosGet<LogPage>("$apiRoot/logs/$page").then { data ->
            setLogs(data.logs)
            setPage(data.page)
            setPages(data.pages)
        }.handleForbidden(history)
    }

    table("table table-sm") {
        thead {
            tr {
                th(classes = "col-2") {
                    +"Time"
                }
                th(classes = "col-1") {
                    +"Type"
                }
                th(classes = "col-1") {
                    +"User"
                }
                th(classes = "col-4") {
                    +"Message"
                }
            }
        }
        tbody {
            logs.forEach { line ->
                tr("table-${line.type.color}") {
                    td {
                        +line.formattedTime()
                    }
                    td {
                        +line.type.human
                    }
                    td {
                        +line.user
                    }
                    td {
                        +line.message
                    }
                }
            }
        }
    }

    nav {
        ul("pagination justify-content-center") {
            li("page-item${if (page > 1) "" else " disabled"}") {
                key = "pagination-previous"
                a("#", classes = "page-link") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()
                        setPage(page - 1)
                    }
                    span { +"\u00ab" }
                }
            }

            // Always show 5 pages if possible
            val range = max(1, min(pages - 4, page - 2))
                .rangeTo(min(pages, max(5, page + 2)))

            range.forEach { idx ->
                li("page-item${if (idx == page) " active" else ""}") {
                    key = "pagination-$idx"
                    a("#", classes = "page-link") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()
                            setPage(idx)
                        }
                        +"$idx"
                    }
                }
            }

            li("page-item${if (page < pages) "" else " disabled"}") {
                key = "pagination-next"
                a("#", classes = "page-link") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()
                        setPage(page + 1)
                    }
                    span { +"\u00bb" }
                }
            }
        }
    }
}
