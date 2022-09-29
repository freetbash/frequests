#include <frequests/frequests.h>
#include <regex>
#include <stdlib.h>
using namespace xsystem;
std::string Frequest::construct_http_header(HTTP_METHOD method,std::string url){
    this->url=url;
    std::string http_method;
    switch (method){
    case HTTP_METHOD::GET:
        http_method="GET";
        break;
    case HTTP_METHOD::POST:
        http_method="POST";
        break;
    case HTTP_METHOD::PUT:
        http_method="PUT";
        break;
    case HTTP_METHOD::HEAD:
        http_method="HEAD";
        break;
    case HTTP_METHOD::DEL:
        http_method="DELETE";
        break;
    default:
        http_method="GET";
        break;
    }

    std::string host;
    std::string path="/";
    bool https=false;
    int port=80;
	std::smatch m;
	std::regex p("(http|HTTP|https|HTTPS):\\/\\/(.*?)\\/?(.*+)");
	if(std::regex_search(url,m,p)){
		std::string t = m.str(1);
        host = m.str(3);
        if(std::regex_search(host,m,std::regex("(.*?)\\/(.*+)"))){
            path+=m.str(2);
        }
        if(std::regex_search(t,std::regex("(https|HTTP)"))){
            https=true;
            port=443;
        }
		if(std::regex_search(host,m,std::regex("(.*?):(\\d+)"))){
            host=m.str(1);
			port=atoi(m.str(2).c_str());
		}
    }else{
        throw std::string(url)+std::string(" is not a url");
    }
    this->set_header("Host",host);
    std::string header;
    header=http_method+" "+path+" "+std::string(HTTP_VERSION)+HTTP_DELIM;
    for(std::pair<std::string,std::string> each:this->headers){
        header+=each.first+": "+each.second+HTTP_DELIM;
    }
    header+=HTTP_DELIM_DOUBLE;
    this->dst_ip=xsystem::net::domain_to_ip(host);
    this->dst_port=port;
    return header;

}

Frequest *const Frequest::set_header(std::string key,std::string value){
    this->headers[key]=value;
    return this;
}

Frequest::Frequest(){
    this->body = (char *)malloc(0);
    this->body_len=0;
    this->set_header("User-Agent",F_USERAGENT)
        ->set_header("Accept","*/*")
        ->set_header("Accept-Encoding","gzip, deflate")
        ->set_header("Connection","close")
    ;
}

Frequest::~Frequest(){
    free(this->body);
}
void Frequest::add_body(char *data,unsigned long len){
    this->body = (char *)realloc(this->body,this->body_len+len);
	int now=this->body_len;// now = 2
	for(int i=0;i<len;i++){
		this->body[now]=data[i];
		now++;
	}
	this->body_len=this->body_len+len;
}
void Frequest::paser_http_response_header(Fresponse *response,std::string http_header){
    std::smatch m;
    if(std::regex_search(http_header,m,std::regex("HTTP/(\\d\\.?\\d) (\\d+) (.*?)\\r\\n"))){
        response->status_code=std::stoi(m.str(2));
        response->reson=m.str(3);
        std::string::const_iterator iterStart = http_header.begin();
        std::string::const_iterator iterEnd   = http_header.end();
        while(std::regex_search(iterStart,iterEnd,m,std::regex("(.*?): (.*?)\\r\\n"))){
            iterStart=m[0].second;
            response->headers.insert(std::pair<std::string,std::string>(m.str(1),m.str(2)));
        }
    }

}

void Frequest::handle_http_response(Fresponse *response){
    response->request=this;
    std::string t;
    char ch;
    int trecv;
    while((trecv=recv(this->fsocket.nfd,&ch,1,0))>0){
        t+=ch;
        if(ch=='\r'){
            trecv=recv(this->fsocket.nfd,&ch,1,0);
            t+=ch;
            if(trecv>0 && ch =='\n'){
                trecv=recv(this->fsocket.nfd,&ch,1,0);
                t+=ch;
                if(trecv>0 && ch =='\r'){
                    trecv=recv(this->fsocket.nfd,&ch,1,0);
                    t+=ch;
                    if(trecv>0 && ch =='\n'){
                        t+=ch;
                        break;
                    }
                }
            }
        }
    }
    response->url=this->url;
    this->paser_http_response_header(response,t);
    char *buff;
    unsigned long len=0;
    if(response->headers.count("Content-Length")){
        len= strtoul(response->headers["Content-Length"].c_str(),NULL,10);
        len+=1;
        buff=(char *)malloc(sizeof(char)*len);
        memset(buff,0,sizeof(char)*len);
        if(recv(this->fsocket.nfd,buff,len,0)<0){
            PRINT_ERROR
        }
        
    }else{
        char temp[MAX_SIZE];
        buff=(char *)malloc(sizeof(char)*MAX_SIZE);
        for(;;){
            if(trecv>0){
                trecv=recv(this->fsocket.nfd,temp,MAX_SIZE,0);
                strcat(buff,temp);
                memset(temp,0,sizeof(char)*MAX_SIZE);
                len+=trecv;
            }else{
                //LOG(std::to_string(i));
                len+=1;
                break;
            }
        }
    }
    buff[len]='\0';
    response->content_len=len-1;
    response->text=std::string(buff);
    response->content=buff;
}


Fresponse Frequest::get(std::string url){
    std::string header = this->construct_http_header(GET,url);
    this->fsocket.Connect(this->dst_ip,this->dst_port);
    this->fsocket.Send(header.c_str());
    this->fsocket.Send(this->body,this->body_len);
    Fresponse response;
    this->handle_http_response(&response);
    this->fsocket.Close();
    return response;
}

Fresponse Frequest::del(std::string url){
    std::string header = this->construct_http_header(DEL,url);
    this->fsocket.Connect(this->dst_ip,this->dst_port);
    this->fsocket.Send(header.c_str());
    this->fsocket.Send(this->body,this->body_len);
    Fresponse response;
    this->handle_http_response(&response);
    this->fsocket.Close();
    return response;

}
Fresponse Frequest::put(std::string url){
    std::string header = this->construct_http_header(PUT,url);
    this->fsocket.Connect(this->dst_ip,this->dst_port);
    this->fsocket.Send(header.c_str());
    this->fsocket.Send(this->body,this->body_len);
    Fresponse response;
    this->handle_http_response(&response);
    this->fsocket.Close();
    return response;

}
Fresponse Frequest::head(std::string url){
    std::string header = this->construct_http_header(HEAD,url);
    this->fsocket.Connect(this->dst_ip,this->dst_port);
    this->fsocket.Send(header.c_str());
    this->fsocket.Send(this->body,this->body_len);
    Fresponse response;
    this->handle_http_response(&response);
    this->fsocket.Close();
    return response;
}
Fresponse Frequest::post(std::string url){
    std::string header = this->construct_http_header(POST,url);
    this->fsocket.Connect(this->dst_ip,this->dst_port);
    this->fsocket.Send(header.c_str());
    this->fsocket.Send(this->body);
    Fresponse response;
    this->handle_http_response(&response);
    this->fsocket.Close();
    return response;
}


Fresponse::Fresponse(){
    this->content = (char *)malloc(0);
}

Fresponse::~Fresponse(){
    
}