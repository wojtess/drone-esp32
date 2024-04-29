.text
.global esp_wifi_80211_tx_mod

.literal_position
.literal .LC0, esp_wifi_80211_tx+20

esp_wifi_80211_tx_mod:
    entry a1, 0x30
    extui a5, a5, 0x0, 0x8
    mov a13, a5
    mov a12, a4
    mov.n a11, a3
    mov.n a10, a2
    movi a10, 0
    l32r a13, .LC0
    jx a13
