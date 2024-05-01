package uk.co.thomasc.tcpdu.page

import external.axiosGet
import kotlinx.datetime.Instant
import kotlinx.serialization.Serializable
import react.Props
import react.dom.table
import react.dom.tbody
import react.dom.td
import react.dom.th
import react.dom.thead
import react.dom.tr
import react.fc
import react.router.useNavigate
import react.useEffectOnce
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.util.EnumAsIntSerializer

@Serializable
data class LogLine(val time: Long, val type: LogType, val user: String, val message: String) {
    fun getInstant() = Instant.fromEpochSeconds(time)
}

@Serializable(with = LogType.LogTypeSerializer::class)
enum class LogType(val enc: Int) {
    OUTLET_STATE(0), DEVICE_IP(1), FIRMWARE(2), CRASH(3);

    class LogTypeSerializer : EnumAsIntSerializer<LogType>(
        "LogType",
        { it.enc },
        { v -> LogType.entries.first { it.enc == v } }
    )
}

@Serializable
data class LogPage(val logs: List<LogLine>, val count: Int, val page: Int, val pages: Int)

val logsPage = fc<Props> {
    val history = useNavigate()
    val (logs, setLogs) = useState(listOf<LogLine>())

    useEffectOnce {
        axiosGet<LogPage>("$apiRoot/logs/1").then {
            setLogs(it.data.logs)
        }.handleForbidden(history)
    }

    table("table table-striped") {
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
                tr {
                    td {
                        +line.getInstant().toString()
                    }
                    td {
                        +line.type.name
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
}
