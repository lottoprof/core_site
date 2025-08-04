#include <string>
#include <functional>
#include <syslog.h>
#include <pthread.h>
#include <chrono>
#include <cstdio>
#include <iomanip>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <cryptopp/base32.h>
#include <cryptopp/algparam.h>

using namespace std;

std::string urldecode(const std::string& input) {
    std::ostringstream decoded;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            // Decode hexadecimal representation
            char hex[3] = { input[i + 1], input[i + 2], '\0' };
            int decodedChar;
            std::istringstream(hex) >> std::hex >> decodedChar;
            decoded << static_cast<char>(decodedChar);
            i += 2; // Move to the next character after '%'
        } else if (input[i] == '+') {
            // Replace '+' with space
            decoded << ' ';
        } else {
            // Copy other characters as is
            decoded << input[i];
        }
    }
    return decoded.str();
}
int calculateHMAC_SHA256( const std::string& data, const unsigned char * key,
                          const int key_len,
                          unsigned char *hmac_buf, 
                          unsigned int &hmac_len) {
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key, key_len, EVP_sha256(), NULL);
    HMAC_Update(ctx, reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
    HMAC_Final(ctx, hmac_buf, &hmac_len);
    HMAC_CTX_free(ctx);
    return 0;
}

int main (int argc, char** argv)
{
   string  initData_s = "query_id=AAFOwc8UAAAAAE7BzxQebvXh&user=%7B%22id%22%3A349159758%2C%22first_name%22%3A%22%40alx%22%2C%22last_name%22%3A%22%22%2C%22language_code%22%3A%22ru%22%2C%22allows_write_to_pm%22%3Atrue%7D&auth_date=1706691888&hash=737cb1d9adaf7ee934cb1106e363a24cf201d856c6b6264686fcc8ca44cbfbba" ;
   std::string post_data = urldecode(initData_s);
    fprintf(stdout, "try to parse === [%s]\n",post_data.c_str());
    map <string, std::string> post_vals;
    std::vector<std::string> dataToCheck;
    int pos = 0;

    while (pos!=string::npos)
    {
        int name_pos = post_data.find_first_not_of('&',pos);
        int value_pos = post_data.find('=',name_pos);
        if(name_pos != std::string::npos && value_pos != std::string::npos)
        {
            std::string name = post_data.substr(name_pos,value_pos - name_pos);
            std::string value = post_data.substr(value_pos+1,post_data.find('&',value_pos+1)-value_pos -1);
            post_vals[name] = value;
            if (name != "hash")
                dataToCheck.push_back(name + "=" + value); 
            fprintf(stdout, "[%s=%s]\n", name.c_str(), value.c_str());
            pos = post_data.find('&',value_pos);
            
        }
        else
        {
            pos = std::string::npos;
        }

    } //while npos

    std::string hash =  post_vals["hash"];
    std::sort(dataToCheck.begin(), dataToCheck.end());
     std::string dataToCheckStr ;
    int it=0; 
    for (auto &a :dataToCheck)
    {
        #if 1
        if (it)
        {
            dataToCheckStr += "\n";
            dataToCheckStr +=a ;
        }
        else
        {
            dataToCheckStr +=a ;

        }
        it++;
        #else
            dataToCheckStr +=a ;
            dataToCheckStr += "\n";
        #endif
        
    }
    // Calculate HMAC-SHA256
    std::string tg_key = "6397870057:AAFzc7GXw8-L9ByhfV-znZooTtr3nyIG_rw";  // Replace with your actual Telegram Bot Token
    unsigned int hmac_len = 0;
    unsigned char hmac_buf[4096]={0x00};
    const char * webappkey = "WebAppData";
    unsigned int webappkey_len = strlen (webappkey);
    calculateHMAC_SHA256(tg_key,(const unsigned char *)webappkey,webappkey_len, 
                                       hmac_buf, hmac_len);
    //std::string secret = calculateHMAC_SHA256(tg_key,);
    fprintf(stdout, "secret: %s\n", hmac_buf);
    unsigned int hmac_len_result = 0;
    unsigned char hmac_buf_result[4096]={0x00};
   
    calculateHMAC_SHA256(dataToCheckStr, hmac_buf, hmac_len,
                        hmac_buf_result, hmac_len_result);
    
    string s_buf = "";
    for (int a = 0; a< hmac_len_result;a++)
    {
        char tmp_buf[10] = {0x00};
        snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", hmac_buf_result[a]);
        s_buf.append(tmp_buf);
    }

    fprintf(stdout, "checked: [%s]\n",dataToCheckStr.c_str());
    fprintf(stdout, "calculated:\t%s\nbased:\t\t%s\n", s_buf.c_str(), hash.c_str());
    std::string session;
    int error=0;
    return 0;
}
