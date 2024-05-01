package uk.co.thomasc.tcpdu.page

import external.axiosGet
import kotlinx.serialization.Serializable
import react.Props
import react.dom.div
import react.fc
import react.router.useNavigate
import react.useEffectOnce
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.page.config.ntpConfig
import uk.co.thomasc.tcpdu.page.config.radiusConfig
import uk.co.thomasc.tcpdu.page.config.wifiConfig

@Serializable
data class PDUConfig(val wifi: WifiConfig, val radius: RadiusConfig, val auth: AuthConfig, val ntp: NtpConfig, val log: LogConfig)

@Serializable
data class WifiConfig(val ssid: String?, val pass: String?)

@Serializable
data class RadiusConfig(val ip: String?, val port: Int?, val secret: String?, val timeout: Int?, val retries: Int?)

@Serializable
data class AuthConfig(val validityPeriod: Int)

@Serializable
data class NtpConfig(val host: String?, val offset: Int?)

@Serializable
data class LogConfig(val serialMask: ULong, val syslogMask: ULong, val emailMask: ULong, val smtp: SmtpConfig)

@Serializable
data class SmtpConfig(val host: String, val port: Int, val user: String, val password: String, val from: String, val to: String)

val configPage = fc<Props> {
    val (config, setConfig) = useState<PDUConfig>()
    val history = useNavigate()

    useEffectOnce {
        axiosGet<PDUConfig>("$apiRoot/config").then {
            setConfig(it.data)
        }.handleForbidden(history)
    }

    if (config != null) {
        div("config row row-cols-1 g-4") {
            val configBlocks = listOf(wifiConfig, radiusConfig, ntpConfig)

            configBlocks.forEach {
                div("col") {
                    it.invoke {
                        attrs.config = config
                    }
                }
            }
        }
    }
}
