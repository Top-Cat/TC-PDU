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
import uk.co.thomasc.tcpdu.page.config.logsConfig
import uk.co.thomasc.tcpdu.page.config.mqttConfig
import uk.co.thomasc.tcpdu.page.config.ntpConfig
import uk.co.thomasc.tcpdu.page.config.radiusConfig
import uk.co.thomasc.tcpdu.page.config.smtpConfig
import uk.co.thomasc.tcpdu.page.config.wifiConfig

@Serializable
data class PDUConfig(val wifi: WifiConfig, val radius: RadiusConfig, val auth: AuthConfig, val ntp: NtpConfig, val log: LogConfig, val mqtt: MqttConfig)

@Serializable
data class WifiConfig(val ssid: String? = null, val pass: String? = null)

@Serializable
data class RadiusConfig(val ip: String? = null, val port: Int? = null, val secret: String? = null, val timeout: Int? = null, val retries: Int? = null)

@Serializable
data class AuthConfig(val validityPeriod: Int? = null)

@Serializable
data class NtpConfig(val host: String? = null, val tz: String? = null)

@Serializable
data class LogConfig(val serialMask: ULong? = null, val syslogMask: ULong? = null, val emailMask: ULong? = null, val smtp: SmtpConfig? = null, val days: Int? = null)

@Serializable
data class SmtpConfig(val host: String? = null, val port: Int? = null, val user: String? = null, val password: String? = null, val from: String? = null, val to: String? = null)

@Serializable
data class MqttConfig(val host: String? = null, val port: Int? = null, val user: String? = null, val password: String? = null, val clientId: String? = null, val prefix: String? = null)

val configPage = fc<Props> {
    val (config, setConfig) = useState<PDUConfig>()
    val history = useNavigate()

    useEffectOnce {
        axiosGet<PDUConfig>("$apiRoot/config").then {
            setConfig(it.data)
        }.handleForbidden(history)
    }

    if (config != null) {
        div("config row row-cols-1 row-cols-md-2 g-4") {
            val configBlocks = mapOf(
                wifiConfig to "col",
                ntpConfig to "col",
                mqttConfig to "col",
                smtpConfig to "col",
                radiusConfig to "col",
                logsConfig to "col" // col-md-12
            )

            configBlocks.forEach { (fc, clazz) ->
                div(clazz) {
                    fc.invoke {
                        attrs.config = config
                    }
                }
            }
        }
    }
}
