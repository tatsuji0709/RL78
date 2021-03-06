//=====================================================================//
/*!	@file
	@brief	タッチ・スイッチのサンプル @n
			・プルアップ抵抗は１Ｍオーム @n
			・タッチパッドは、１０ｍｍ四方 @n
			・タッチパッドは、ポリイミドテープなどで絶縁する
    @author 平松邦仁 (hira@rvf-rc45.net)
	@copyright	Copyright (C) 2016 Kunihito Hiramatsu @n
				Released under the MIT license @n
				https://github.com/hirakuni45/RL78/blob/master/LICENSE
*/
//=====================================================================//
#include "common/renesas.hpp"
#include "common/port_utils.hpp"
#include "common/fifo.hpp"
#include "common/uart_io.hpp"
#include "common/format.hpp"
#include "common/itimer.hpp"
#include "common/delay.hpp"

namespace {

	// 送信、受信バッファの定義
	typedef utils::fifo<uint8_t, 16> buffer;
	// UART1 の定義（SAU2、SAU3）
	device::uart_io<device::SAU02, device::SAU03, buffer, buffer> uart_;

	device::itimer<uint8_t> itm_;

}


extern "C" {

	void sci_putch(char ch)
	{
		uart_.putch(ch);
	}


	void sci_puts(const char* str)
	{
		uart_.puts(str);
	}


	char sci_getch(void)
	{
		return uart_.getch();
	}


	uint16_t sci_length()
	{
		return uart_.recv_length();
	}


	void UART1_TX_intr(void)
	{
		uart_.send_task();
	}


	void UART1_RX_intr(void)
	{
		uart_.recv_task();
	}


	void UART1_ER_intr(void)
	{
		uart_.error_task();
	}


	void ITM_intr(void)
	{
		itm_.task();
	}
};


namespace {

	uint16_t count_active_()
	{
		uint16_t n = 0;
		device::PM2.B2 = 1;  // input
		do {
			++n;
		} while(device::P2.B2() == 0) ;
		device::PM2.B2 = 0;  // output
		device::P2.B2 = 0;  // 仮想コンデンサをショートしてリセット
		return n;
	}

}


int main(int argc, char* argv[])
{
	utils::port::pullup_all();  ///< 安全の為、全ての入力をプルアップ

	device::PM4.B3 = 0;  // output

	{
		// 割り込みを使う場合、intr_level を設定
		// ポーリングなら「０」
		uint8_t intr_level = 1;
		uart_.start(115200, intr_level);
	}

	// インターバルタイマー設定
	{
		uint8_t intr_level = 1;
		itm_.start(60, intr_level);
	}

	utils::format("Start RL78/G13 touch switch sample\n");

	uint16_t touch_ref = 0;
	//　タッチ入力ポートの設定
	// プルアップ設定がある場合は、OFF にする。
	{
		device::ADPC = 0x01; // All A/D chanel for digital I/O
		device::PM2.B2 = 0;  // 初期は出力にする

		uint16_t sum = 0;
		for(uint8_t i = 0; i < 30; ++i) {
			itm_.sync();
			sum += count_active_();
		}
		touch_ref = sum / 30;
		utils::format("Touch refarence: %d\n") % touch_ref;
	}

	bool touch = false;
	bool touch_back = false;
	uint16_t touch_sum = 0;
	while(1) {
		itm_.sync();

		{
			auto ref = count_active_();

			if(ref >= (touch_ref + 10)) {
///				utils::format("%d\n") % static_cast<int>(ref);
				touch_sum += ref;
				if(touch_sum >= 1000) {
					touch = true;
				}
			} else {
				touch = false;
				touch_sum = 0;
			}

			if(touch && !touch_back) {
				utils::format("Touch on\n");
			}
			if(!touch && touch_back) {
				utils::format("Touch off\n");
			}
			touch_back = touch;

			device::P4.B3 = !touch;
		}
	}
}
