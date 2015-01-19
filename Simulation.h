#pragma once

#include <SDL2\SDL_net.h>
#include <iosfwd>
#include <vector>
#include <unordered_map>
#include <array>
#include <map>
#include <GL\Vectors.h>

namespace ff {
	namespace sim {
		namespace data {
			using namespace sim::enums;
			struct Unit {
				std::unordered_map<int, int> data;
				void x() {
					data.reserve(100 * 100);
				}
			};
			struct Fire {
			};
			struct Land {
			};
			struct Object {
			};
			struct Weather {
			};

		}
		namespace observation {
			using namespace enums;
			/*knows everything in the past*/
			struct Archiver {
				void* history;
				void store(void*);
				void retrieve(void*);
			};
			/*knows everything current*/
			class Server {
				std::vector<data::Unit> units;
				std::vector<data::Fire> fires;
			public:
				void update();
			};
			/*knows only what it has witnessed*/
			struct User {
				void* memory;
				void get(void*);
				void send(void*);
			};
		}
	}
}

namespace ff {
	namespace sim {
		namespace enums {
			bool hasFlag(unsigned int value, unsigned int flag) {
				return (value & flag) > 0;
			}
			enum Direction {
				DIR_NONE = 0,
				DIR_UP = 1<<0,
				DIR_LEFT = 1<<1,
				DIR_DOWN = 1<<2,
				DIR_RIGHT = 1<<3,
				DIR_OUT = 1<<4,
				DIR_IN = 1<<5
			};
			enum ObserverEnum {
				FFO_NULL,
				FFO_ARCHIVER,
				FFO_SERVER,
				FFO_USER,
				FFO_NUM
			};
			enum ArchiveEnum {
				FFA_NULL,
				FFA_NUM
			};
			enum ServerEnum {
				FFS_NO_LAYERS		= 0,
				FFS_GEOGRAPHY_LAYER	= 1 << 0,
				FFS_OBJECT_LAYER	= 1 << 1,
				FFS_FIRE_LAYER		= 1 << 2,
				FFS_UNIT_LAYER		= 1 << 3,
				FFS_WEATHER_LAYER	= 1 << 4,
				FFS_END_LAYER		= 1 << 5, /*Here only to describe the value of the last layer*/
				FFS_ALL_LAYERS		= FFS_END_LAYER-1,
				FFS_STATIC_LAYERS	= FFS_GEOGRAPHY_LAYER | FFS_OBJECT_LAYER,
				FFS_DYNAMIC_LAYERS	= FFS_FIRE_LAYER | FFS_WEATHER_LAYER | FFS_UNIT_LAYER,
				FFS_FIRESIM_LAYERS	= FFS_FIRE_LAYER | FFS_GEOGRAPHY_LAYER | FFS_OBJECT_LAYER,
			};
			enum UserEnum {
				FFU_NULL,
				FFU_NUM
			};
			enum NetEnum {
				FFNET_NULL,
				FFNET_TCP,
				FFNET_UDP,
				FFNET_IDLE,
				FFNET_CONNECTED,
				FFNET_CONNECTION_PROBLEM,
				FFNET_NUM
			};
		}
	}
}

namespace FF {
	namespace communication {
		using NetHandle = net::Link*;
		namespace net {
			using namespace ff::sim::enums;
			using message = std::vector<char>;
			struct SimRequest {
				enum ContentIdentifiers {
					LAYER_C,
					END
				};
				unsigned char layers = FFS_ALL_LAYERS;
				const char end = 0xFF;
			};
			struct SimContract {
				//to participate in the network, you need to be prepared to send a request
				virtual message createSimRequest() = 0;
				virtual void getRequested(message& data) = 0;
			};
			struct Link {

			};
		}
	}
}