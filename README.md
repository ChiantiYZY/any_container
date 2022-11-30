# any_container
Recently I'm studying the mordern C++ and noticed the C++ 17 new feature `std::any`. It's such a very powerful tool, but it's internal implementation is really complicated. 
I didn't see a really good and intuitive implementation on the website, so I decided to make it myself. 
This practice only achieves very basic `std::any` functionalities: move/copy and `get()`. The implementation is not perfect, but should give you a brief idea about how `std::any` is implemented. 
