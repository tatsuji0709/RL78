//=====================================================================//
/*!	@file
	@brief	A/D 変換による複数スイッチ検出サンプル @n
			P22/ANI2(54) ４つのスイッチ（排他）@n
                      P22                           @n
                       |                            @n
			VDD - 10K -+- 3.3K -+- 6.8K -+- 20K -+  @n
                       |        |        |       |  @n
			         RIGHT     UP      DOWN    LEFT @n
                       |        |        |       |  @n
                      VSS      VSS      VSS     VSS @n
			P23/ANI3(55)：２つのスイッチ（同時押し対応）@n
            VDD - 5K -+- 10K -+- 5K -+- VSS @n
                      |       |      |      @n
                      +-  A  -+-  B -+
	@author	平松邦仁 (hira@rvf-rc45.net)
*/
//=====================================================================//
#include <cstdint>
#include "common/port_utils.hpp"
#include "common/fifo.hpp"
#include "common/uart_io.hpp"
#include "common/format.hpp"
#include "common/itimer.hpp"
#include "common/adc_io.hpp"
#include "common/task.hpp"
#include "common/bitset.hpp"
#include "common/switch_man.hpp"

namespace {

	typedef utils::fifo<uint8_t, 32> buffer;
	device::uart_io<device::SAU02, device::SAU03, buffer, buffer> uart_;

	device::itimer<uint8_t> itm_;

	// A/D 変換の終了チャネル数を設定
	typedef device::adc_io<4, utils::null_task> adc;
	adc adc_;

	enum class SWITCH : uint8_t {
		RIGHT,
		UP,
		DOWN,
		LEFT,
		A,
		B
	};

	typedef utils::bitset<uint8_t, SWITCH> switch_bits;
	utils::switch_man<switch_bits> switch_man_;
}

/// 割り込みベクターの定義
const void* ivec_[] __attribute__ ((section (".ivec"))) = {
	/*  0 */  nullptr,
	/*  1 */  nullptr,
	/*  2 */  nullptr,
	/*  3 */  nullptr,
	/*  4 */  nullptr,
	/*  5 */  nullptr,
	/*  6 */  nullptr,
	/*  7 */  nullptr,
	/*  8 */  nullptr,
	/*  9 */  nullptr,
	/* 10 */  nullptr,
	/* 11 */  nullptr,
	/* 12 */  nullptr,
	/* 13 */  nullptr,
	/* 14 */  nullptr,
	/* 15 */  nullptr,
	/* 16 */  reinterpret_cast<void*>(uart_.send_task),  // UART1-TX
	/* 17 */  reinterpret_cast<void*>(uart_.recv_task),  // UART1-RX
	/* 18 */  reinterpret_cast<void*>(uart_.error_task), // UART1-ER
	/* 19 */  nullptr,
	/* 20 */  nullptr,
	/* 21 */  nullptr,
	/* 22 */  nullptr,
	/* 23 */  nullptr,
	/* 24 */  reinterpret_cast<void*>(adc_.task),
	/* 25 */  nullptr,
	/* 26 */  reinterpret_cast<void*>(itm_.task),
};


extern "C" {
	void sci_putch(char ch)
	{
		uart_.putch(ch);
	}

	void sci_puts(const char* str)
	{
		uart_.puts(str);
	}
};


int main(int argc, char* argv[])
{
	utils::port::pullup_all();  ///< 安全の為、全ての入力をプルアップ

	device::PM4.B3 = 0;  // output

	{
		uint8_t intr_level = 1;
		uart_.start(115200, intr_level);
	}

	{
		uint8_t intr_level = 1;
		itm_.start(60, intr_level);
	}

	{
		device::PM2.B2 = 1;
		device::PM2.B3 = 1;
		uint8_t intr_level = 1;
		adc_.start(adc::REFP::VDD, adc::REFM::VSS, intr_level);
	}

	uart_.puts("Start RL78/G13 A/D Switch sample\n");

	uint8_t n = 0;
	while(1) {
		itm_.sync();
		adc_.start_scan(2);  // チャネル２から開始

		adc_.sync();

		// ４つのスイッチ判定（排他的）
		auto val = adc_.get(2);
		val >>= 6;   // 0 to 1023
		val += 128;  // 閾値のオフセット（1024 / 4(SWITCH) / 2）
		val /= 256;  // デコード（1024 / 4(SWITCH）

		switch_bits tmp;
		if(val < 4) {
			tmp.set(static_cast<SWITCH>(val));
		}

		// ２つのスイッチ判定（同時押し判定）
		val = adc_.get(3);
		val >>= 6;  // 0 to 1023
		if(val < 256) {
			tmp.set(SWITCH::A);
			tmp.set(SWITCH::B);
		} else if(val < 594) {
			tmp.set(SWITCH::A);
		} else if(val < 722) {
			tmp.set(SWITCH::B);
		}

		switch_man_.service(tmp);

		if(switch_man_.get_positive().get(SWITCH::UP)) {
			utils::format("UP   : on\n");
		}
		if(switch_man_.get_positive().get(SWITCH::DOWN)) {
			utils::format("DOWN : on\n");
		}
		if(switch_man_.get_positive().get(SWITCH::LEFT)) {
			utils::format("LEFT : on\n");
		}
		if(switch_man_.get_positive().get(SWITCH::RIGHT)) {
			utils::format("RIGHT: on\n");
		}

		if(switch_man_.get_positive().get(SWITCH::A)) {
			utils::format("A    : on\n");
		}
		if(switch_man_.get_positive().get(SWITCH::B)) {
			utils::format("B    : on\n");
		}

		if(switch_man_.get_negative().get(SWITCH::UP)) {
			utils::format("UP   : off\n");
		}
		if(switch_man_.get_negative().get(SWITCH::DOWN)) {
			utils::format("DOWN : off\n");
		}
		if(switch_man_.get_negative().get(SWITCH::LEFT)) {
			utils::format("LEFT : off\n");
		}
		if(switch_man_.get_negative().get(SWITCH::RIGHT)) {
			utils::format("RIGHT: off\n");
		}

		if(switch_man_.get_negative().get(SWITCH::A)) {
			utils::format("A    : off\n");
		}
		if(switch_man_.get_negative().get(SWITCH::B)) {
			utils::format("B    : off\n");
		}

		++n;
		if(n >= 30) {
			n = 0;
		}
		device::P4.B3 = n < 10 ? false : true; 	
	}
}
