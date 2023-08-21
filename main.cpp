#include "box.hpp"
#include "image.hpp"
#include "scrollableFrame.hpp"
#include "text.hpp"
#include "window.hpp"



int main(int, char **) {
	using namespace squi;
	squi::Window window;
	{
		Image::Data img = Image::Data::fromUrl("https://interactive-examples.mdn.mozilla.net/media/examples/plumeria-146x200.jpg");
		window.addChild(ScrollableFrame{
			.children{
				Text{
					.text{"Fit: none"},
				},
				Box{
					.widget{
						.width = 400.f,
						.height = 400.f,
						.margin{8.f},
					},
					.color{0xFFFFFF22},
					.child{
						Image{
							.fit = Image::Fit::none,
							.image{img},
						},
					},
				},
				  Text{
					  .text{"Fit: fill"},
				  },
				  Box{
					  .widget{
						  .width = 400.f,
						  .height = 400.f,
						  .margin{8.f},
					  },
					  .color{0xFFFFFF22},
					  .child{
						  Image{
							  .fit = Image::Fit::fill,
							  .image{img},
						  },
					  },
				  },
				  Text{
					  .text{"Fit: cover"},
				  },
				  Box{
					  .widget{
						  .width = 400.f,
						  .height = 400.f,
						  .margin{8.f},
					  },
					  .color{0xFFFFFF22},
					  .child{
						  Image{
							  .fit = Image::Fit::cover,
							  .image{img},
						  },
					  },
				  },
				  Text{
					  .text{"Fit: contain"},
				  },
				  Box{
					  .widget{
						  .width = 400.f,
						  .height = 400.f,
						  .margin{8.f},
					  },
					  .color{0xFFFFFF22},
					  .child{
						  Image{
							  .fit = Image::Fit::contain,
							  .image{img},
						  },
					  },
				  },
			},
		});
	}
	window.run();
	return 0;
}
