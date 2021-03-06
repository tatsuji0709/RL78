#pragma once
//=====================================================================//
/*!	@file
	@brief	RL78 文字列操作ユーティリティー
    @author 平松邦仁 (hira@rvf-rc45.net)
	@copyright	Copyright (C) 2017 Kunihito Hiramatsu @n
				Released under the MIT license @n
				https://github.com/hirakuni45/RL78/blob/master/LICENSE
*/
//=====================================================================//
#include <cstring>
#include "ff12a/src/ff.h"
#include "common/time.h"

namespace utils {

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief  文字列操作クラス
	*/
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	struct str {

		//-----------------------------------------------------------------//
		/*!
			@brief  １６進数ダンプ
			@param[in]	src	ソース
			@param[in]	num	数
			@param[in]	lin	１行辺りの表示数
		*/
		//-----------------------------------------------------------------//
		static void hex_dump(char* src, uint16_t num, uint8_t lin)
		{
			uint8_t l = 0;
			for(uint16_t i = 0; i < num; ++i) {
				auto n = *src++;
				utils::format("%02X") % static_cast<uint16_t>(n);
				++l;
				if(l == lin) {
					utils::format("\n");
					l = 0;
				} else {
					utils::format(" ");
				}
			}
		}


		//-----------------------------------------------------------------//
		/*!
			@brief  UTF-8 から UTF-16 への変換
			@param[in]	src	ソース
			@param[in]	dst	変換先
		*/
		//-----------------------------------------------------------------//
		static void utf8_to_utf16(const char* src, WCHAR* dst)
		{
			uint8_t cnt = 0;
			uint16_t code = 0;
			char tc;
			while((tc = *src++) != 0) {
				uint8_t c = static_cast<uint8_t>(tc);
				if(c < 0x80) { code = c; cnt = 0; }
				else if((c & 0xf0) == 0xe0) { code = (c & 0x0f); cnt = 2; }
				else if((c & 0xe0) == 0xc0) { code = (c & 0x1f); cnt = 1; }
				else if((c & 0xc0) == 0x80) {
					code <<= 6;
					code |= c & 0x3f;
					cnt--;
					if(cnt == 0 && code < 0x80) {
						code = 0;	// 不正なコードとして無視
						break;
					} else if(cnt < 0) {
						code = 0;
					}
				}
				if(cnt == 0 && code != 0) {
					*dst++ = code;
					code = 0;
				}
			}
			*dst = 0;
		}


		//-----------------------------------------------------------------//
		/*!
			@brief  UTF-16 から UTF-8 への変換
			@param[in]	code	UTF-16 コード
			@param[in]	dst	変換先
			@return 変換先の最終ポインター
		*/
		//-----------------------------------------------------------------//
		static char* utf16_to_utf8(uint16_t code, char* dst)
		{
			if(code < 0x0080) {
				*dst++ = code;
			} else if(code >= 0x0080 && code <= 0x07ff) {
				*dst++ = 0xc0 | ((code >> 6) & 0x1f);
				*dst++ = 0x80 | (code & 0x3f);
			} else if(code >= 0x0800) {
				*dst++ = 0xe0 | ((code >> 12) & 0x0f);
				*dst++ = 0x80 | ((code >> 6) & 0x3f);
				*dst++ = 0x80 | (code & 0x3f);
			}
			return dst;
		}


		//-----------------------------------------------------------------//
		/*!
			@brief  UTF-16 から UTF-8 への変換
			@param[in]	src	ソース
			@param[in]	dst	変換先
		*/
		//-----------------------------------------------------------------//
		static void utf16_to_utf8(const WCHAR* src, char* dst) 
		{
			uint16_t code;
			while((code = static_cast<uint16_t>(*src++)) != 0) {
				dst = utf16_to_utf8(code, dst);
			}
			*dst = 0;
		}

#if _USE_LFN != 0
		//-----------------------------------------------------------------//
		/*!
			@brief  sjis から UTF-8 への変換
			@param[in]	src	ソース
			@param[in]	dst	変換先
		*/
		//-----------------------------------------------------------------//
		static void sjis_to_utf8(const char* src, char* dst)
		{
			if(src == nullptr) return;
			uint16_t wc = 0;
			char ch;
			while((ch = *src++) != 0) {
				uint8_t c = static_cast<uint8_t>(ch);
				if(wc) {
					if(0x40 <= c && c <= 0x7e) {
						wc <<= 8;
						wc |= c;
						dst = utf16_to_utf8(ff_convert(wc, 1), dst);
					} else if(0x80 <= c && c <= 0xfc) {
						wc <<= 8;
						wc |= c;
						dst = utf16_to_utf8(ff_convert(wc, 1), dst);
					}
					wc = 0;
				} else {
					if(0x81 <= c && c <= 0x9f) wc = c;
					else if(0xe0 <= c && c <= 0xfc) wc = c;
					else {
						dst = utf16_to_utf8(ff_convert(c, 1), dst);
					}
				}
			}
			*dst = 0;
		}


		//-----------------------------------------------------------------//
		/*!
			@brief  UTF-8 から sjis への変換
			@param[in]	src	ソース
			@param[in]	dst	変換先
		*/
		//-----------------------------------------------------------------//
		static void utf8_to_sjis(const char* src, char* dst)
		{
			int8_t cnt = 0;
			uint16_t code = 0;
			char tc;
			while((tc = *src++) != 0) {
				uint8_t c = static_cast<uint8_t>(tc);
				if(c < 0x80) { *dst++ = tc; code = 0; }
				else if((c & 0xf0) == 0xe0) { code = (c & 0x0f); cnt = 2; }
				else if((c & 0xe0) == 0xc0) { code = (c & 0x1f); cnt = 1; }
				else if((c & 0xc0) == 0x80) {
					code <<= 6;
					code |= c & 0x3f;
					cnt--;
					if(cnt == 0 && code < 0x80) {
						code = 0;	// 不正なコードとして無視
						break;
					} else if(cnt < 0) {
						code = 0;
					}
				}
				if(cnt == 0 && code != 0) {
					auto wc = ff_convert(code, 0);
					*dst++ = static_cast<char>(wc >> 8);
					*dst++ = static_cast<char>(wc & 0xff);
					code = 0;
				}			
			}
			*dst = 0;
		}
#endif

		//-----------------------------------------------------------------//
		/*!
			@brief	FatFS が使う時間取得関数
			@return	FatFS 形式の時刻ビットフィールドを返す。@n
			        現在のローカル・タイムがDWORD値にパックされて返されます。@n
					ビット・フィールドは次に示すようになります。@n
					bit31:25 ---> 1980年を起点とした年が 0..127 で入ります。@n
					bit24:21 ---> 月が 1..12 の値で入ります。@n
					bit20:16 ---> 日が 1..31 の値で入ります。@n
					bit15:11 ---> 時が 0..23 の値で入ります。@n
					bit10:5 ---> 分が 0..59 の値で入ります。@n
					bit4:0 ---> 秒/2が 0..29 の値で入ります。
		*/
		//-----------------------------------------------------------------//
		static DWORD get_fattime(time_t t)
		{
//			t = 0;	/* 1970 01-01 00:00:00 */
			struct tm *tp = localtime(&t);

#ifdef _DEBUG
			format("get_fattime: (source) %4d/%d/%d/ %02d:%02d:%02d\n")
				% (tp->tm_year + 1900) % (tp->tm_mon + 1) % tp->tm_mday
				% tp->tm_hour % tp->tm_min % tp->tm_sec;
#endif

			DWORD tt = 0;
			tt |= (DWORD)(tp->tm_sec) >> 1;
			tt |= (DWORD)tp->tm_min  << 5;
			tt |= (DWORD)tp->tm_hour << 11;

			tt |= (DWORD)tp->tm_mday << 16;
			tt |= ((DWORD)(tp->tm_mon) + 1) << 21;
			if(tp->tm_year >= 80) {
				tt |= ((DWORD)(tp->tm_year) - 80) << 25;
			}

			return tt;
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	FatFS が扱う時間値から、time_t への変換
			@param[in]	date	日付
			@param[in]	time	時間
			@return 標準時間値
		*/
		//-----------------------------------------------------------------//
		static time_t fatfs_time_to(WORD date, WORD time)
		{
			struct tm ttm;
/* ftime
			ファイルの変更された時刻、またはディレクトリの作成された時刻が格納されます。
			bit15:11 ---> 時が 0..23 の値で入ります。
			bit10:5  ---> 分が 0..59 の値で入ります。
			bit4:0   ---> 秒/2が 0..29 の値で入ります。
*/
			ttm.tm_sec  =  time & 0x1f;
			ttm.tm_min  = (time >> 5) & 0x3f;
			ttm.tm_hour = (time >> 11) & 0x1f;

/* fdate
			ファイルの変更された日付、またはディレクトリの作成された日付が格納されます。
			bit15:9 ---> 1980年を起点とした年が 0..127 で入ります。
			bit8:5  ---> 月が 1..12 の値で入ります。
			bit4:0  ---> 日が 1..31 の値で入ります。
*/
			ttm.tm_mday =  date & 0x1f;
			ttm.tm_mon  = ((date >> 5) & 0xf) - 1;
			ttm.tm_year = ((date >> 9) & 0x7f) + 1980 - 1900;

			return mktime(&ttm);
		}
	};
}
