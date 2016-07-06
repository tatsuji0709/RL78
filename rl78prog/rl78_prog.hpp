#pragma once
//=====================================================================//
/*!	@file
	@brief	RL78 programmer クラス
	@author	平松邦仁 (hira@rvf-rc45.net)
*/
//=====================================================================//
#include "rl78_protocol.hpp"
#include <boost/format.hpp>

namespace rl78 {
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief	rl78_prog クラス
	 */
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	class prog {
		bool		verbose_;
		protocol	proto_;

		std::string out_section_(uint32_t n, uint32_t num) const {
			return (boost::format("#%02d/%02d: ") % n % num).str();
		}

	public:
		//-------------------------------------------------------------//
		/*!
			@brief	コンストラクター
		*/
		//-------------------------------------------------------------//
		prog(bool verbose = false) : verbose_(verbose) { }


		//-------------------------------------------------------------//
		/*!
			@brief	接続速度を変更する
			@param[in]	path	シリアル・デバイス・パス
			@param[in]	brate	ボーレート
			@param[in]	t		CPU 設定
			@return エラー無ければ「true」
		*/
		//-------------------------------------------------------------//
		bool start(const std::string& path, uint32_t brate, const rl78::protocol::rl78_t& t) {
#if 0
			// 開始
			if(!proto_.start(path)) {
				std::cerr << "Can't open path: '" << path << "'" << std::endl;
				return false;
			}

			// コネクション
			{
				if(!proto_.connection()) {
					proto_.end();
					std::cerr << "Can't connection." << std::endl;
					return false;
				}
				if(verbose_) {
					auto sect = out_section_(1, 1);
					std::cout << sect << "Connection OK." << std::endl;
				}
			}

			// サポート・デバイス問い合わせ
			{
				if(!proto_.inquiry_device()) {
					proto_.end();
					std::cerr << "Inquiry device error." << std::endl;
					return false;
				}
				auto as = proto_.get_device();
				if(verbose_) {
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()));
					}
				}
				// デバイス選択
				if(!proto_.select_device(as[0].code_)) {
					proto_.end();
					std::cerr << "Select device error." << std::endl;
					return false;
				}
			}

			// クロック・モード問い合わせ
			{
				if(!proto_.inquiry_clock_mode()) {
					proto_.end();
					std::cerr << "Inquiry clock-mode error." << std::endl;
					return false;
				}
				auto as = proto_.get_clock_mode();
				if(verbose_) {
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()));
					}				
				}
				// クロック・モード選択
				if(!proto_.select_clock_mode(as[0])) {
					proto_.end();
					std::cerr << "Select clock-mode error." << std::endl;
					return false;
				}
			}

			// 逓倍比問い合わせ
			{
				if(!proto_.inquiry_multiplier()) {
					proto_.end();
					std::cerr << "Inquiry multiplier error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto as = proto_.get_multiplier();
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()));
					}				
				}
			}

			// 動作周波数問い合わせ
			{
				if(!proto_.inquiry_frequency()) {
					proto_.end();
					std::cerr << "Inquiry frequency error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto as = proto_.get_frequency();
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()));
					}				
				}
			}

			// ボーレート変更
			{
				if(!proto_.change_speed(ins, brate)) {
					std::cerr << "Can't change speed." << std::endl;
					return false;
				}
				if(verbose_) {
					auto sect = out_section_(1, 1);
					std::cout << sect << "Change baud rate: " << std::endl;
				}
			}

			// ユーザー・ブート領域問い合わせ
			{
				if(!proto_.inquiry_boot_area()) {
					proto_.end();
					std::cerr << "Inquiry boot-area error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto as = proto_.get_boot_area();
					int i = 0;
					for(auto a : as) {
						++i;
						
						a.info(out_section_(i, as.size()) + "Boot ");
					}				
				}
			}

			// ユーザー領域問い合わせ
			{
				if(!proto_.inquiry_area()) {
					proto_.end();
					std::cerr << "Inquiry area error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto as = proto_.get_area();
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()));
					}				
				}
			}

			// ブロック情報問い合わせ
			{
				if(!proto_.inquiry_block()) {
					proto_.end();
					std::cerr << "Inquiry block error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto as = proto_.get_block();
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()));
					}				
				}
			}

			// プログラム・サイズ問い合わせ
			{
				if(!proto_.inquiry_prog_size()) {
					proto_.end();
					std::cerr << "Inquiry prog-size error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto sz = proto_.get_prog_size();
					auto sect = out_section_(1, 1);
					std::cout << sect << (boost::format("Program size: %04X") % sz) << std::endl;
				}
			}

			// データ量域の有無問い合わせ
			{
				if(!proto_.inquiry_data()) {
					proto_.end();
					std::cerr << "Inquiry data error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto sect = out_section_(1, 1);
					std::cout << sect << "Data area: ";
					if(proto_.get_data()) {
						std::cout << "true" << std::endl;
					} else {
						std::cout << "false" << std::endl;
					}
				}
			}

			// データ量域情報問い合わせ
			{
				if(!proto_.inquiry_data_area()) {
					proto_.end();
					std::cerr << "Inquiry data-area error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto as = proto_.get_data_area();
					int i = 0;
					for(auto a : as) {
						++i;
						a.info(out_section_(i, as.size()) + "Data ");
					}				
				}
			}

			// P/E ステータスに移行
			{
				if(!proto_.turn_pe_status()) {
					proto_.end();
					std::cerr << "P/E status error." << std::endl;
					return false;
				}
				if(verbose_) {
					auto sect = out_section_(1, 1);
					std::cout << sect << "ID Protect: ";
					if(proto_.get_protect()) {
						std::cout << "true" << std::endl;
					} else {
						std::cout << "false" << std::endl;
					}					
				}
			}
#endif
			return true;
		}


		//-------------------------------------------------------------//
		/*!
			@brief	リード
			@param[in]	adr	開始アドレス
			@param[out]	dst	書き込みアドレス
			@param[in]	len	読み出しサイズ
			@return 成功なら「true」
		*/
		//-------------------------------------------------------------//
		bool read(uint32_t adr, uint8_t* dst, uint32_t len) {
			if(!proto_.read(adr, len, dst)) {
				proto_.end();
				std::cerr << "Read error." << std::endl;
				return false;
			}
			return true;
		}


		//-------------------------------------------------------------//
		/*!
			@brief	ベリファイ
			@param[in]	adr	開始アドレス
			@param[in]	src	書き込みアドレス
			@param[in]	len	読み出しサイズ
			@return 成功なら「true」
		*/
		//-------------------------------------------------------------//
		bool verify(uint32_t adr, const uint8_t* src, uint32_t len) {
			std::vector<uint8_t> dev;
			dev.resize(len);
			if(!read(adr, &dev[0], len)) {
				return false;
			}
			uint32_t errcnt = 0;
			for(auto d : dev) {
				auto m = *src++;
				if(d != m) {
					++errcnt;
					if(verbose_) {
						std::cerr << (boost::format("0x%08X: D(%02X) to M(%02X)") % adr %
							static_cast<uint32_t>(d) % static_cast<uint32_t>(m)) << std::endl;
					}
				}
				++adr;
			}
			if(errcnt > 0) {
				std::cerr << "Verify error: " << errcnt << std::endl;
				return false;
			}
			return true;
		}


		//-------------------------------------------------------------//
		/*!
			@brief	ライト開始
			@param[in]	data	「true」ならデータ領域
			@return 成功なら「true」
		*/
		//-------------------------------------------------------------//
		bool start_write(bool data) {
			if(!proto_.select_write_area(data)) {
				proto_.end();
				std::cerr << "Write start error.(first)" << std::endl;
				return false;
			}
			return true;
		}


		//-------------------------------------------------------------//
		/*!
			@brief	ライト（２５６バイト）
			@param[in]	adr	開始アドレス
			@param[in]	src	書き込みアドレス
			@return 成功なら「true」
		*/
		//-------------------------------------------------------------//
		bool write(uint32_t adr, const uint8_t* src) {
			if(!proto_.write_page(adr, src)) {
				proto_.end();
				std::cerr << "Write body error." << std::endl;
				return false;
			}
			return true;
		}


		//-------------------------------------------------------------//
		/*!
			@brief	ライト終了
			@return 成功なら「true」
		*/
		//-------------------------------------------------------------//
		bool final_write() {
			if(!proto_.write_page(0xffffffff, nullptr)) {
				proto_.end();
				std::cerr << "Write final error. (fin)" << std::endl;
				return false;
			}
			return true;
		}


		//-------------------------------------------------------------//
		/*!
			@brief	終了
		*/
		//-------------------------------------------------------------//
		void end() {
			proto_.end();
		}
	};
}