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
import uk.co.thomasc.tcpdu.page.system.ethStatus
import uk.co.thomasc.tcpdu.page.system.sysStatus
import uk.co.thomasc.tcpdu.page.system.wifiStatus

@Serializable
data class PDUSystem(
    val power: Float,
    val fw: String = "Unknown",
    val mem: Long,
    val uptime: Long,
    val time: String,
    val offset: Int,
    val wifi: WifiState,
    val eth: EthState,
    val fs: FsState
)

@Serializable
data class FsState(
    val used: Long,
    val total: Long
)

@Serializable
data class WifiState(
    override val connected: Boolean,
    override val mac: String,
    override val ip: String? = null,
    override val dns: String? = null,
    override val gateway: String? = null,
    override val subnet: String? = null,
    val rssi: Int? = null
) : INetworkState

@Serializable
data class EthState(
    override val connected: Boolean,
    override val mac: String,
    override val ip: String? = null,
    override val dns: String? = null,
    override val gateway: String? = null,
    override val subnet: String? = null,
    val fullDuplex: Boolean? = null,
    val linkSpeed: Int? = null
) : INetworkState

interface INetworkState {
    val connected: Boolean
    val mac: String
    val ip: String?
    val dns: String?
    val gateway: String?
    val subnet: String?
}

val systemPage = fc<Props> {
    val history = useNavigate()
    val (system, setSystem) = useState<PDUSystem>()

    useEffectOnce {
        axiosGet<PDUSystem>("$apiRoot/system").then {
            setSystem(it)
        }.handleForbidden(history)
    }

    if (system != null) {
        div("config row row-cols-1 row-cols-md-3 g-4") {
            val systemBlocks = listOf(wifiStatus, ethStatus, sysStatus)

            systemBlocks.forEach {
                div("col") {
                    it.invoke {
                        attrs.system = system
                    }
                }
            }
        }
    }
}
