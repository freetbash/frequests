#include <frequests/frequests.h>
int main(){
        Frequest f;
        auto c = f.get("http://172.28.23.100:3147/chameleon");
        std::cout << c.content << std::endl;
        return 0;
}