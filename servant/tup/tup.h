﻿/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef _WUP_H_
#define _WUP_H_
#include <map>
#include <string>
#include <vector>
#include <sstream>

//支持iphone
#ifdef __APPLE__
    #include "RequestF.h"
#elif defined ANDROID  // android
    #include "RequestF.h"
#else
    #include "tup/RequestF.h"
#endif

// #ifdef __GNUC__
// #   if __GNUC__ >3 || __GNUC_MINOR__ > 3
// #       include <ext/pool_allocator.h>
// #   endif
// #endif
namespace tup
{

//存放tars返回值的key
const std::string STATUS_RESULT_CODE = "STATUS_RESULT_CODE";
const std::string STATUS_RESULT_DESC = "STATUS_RESULT_DESC"; 

/////////////////////////////////////////////////////////////////////////////////
// 属性封装类

template<typename TWriter = tars::BufferWriter, typename TReader = tars::BufferWriter,template<typename> class Alloc = std::allocator >
        //template<typename> class Alloc = __gnu_cxx::__pool_alloc >
class UniAttribute
{
    typedef std::vector<char,Alloc<char> > VECTOR_CHAR_TYPE;
    typedef std::map<std::string, VECTOR_CHAR_TYPE, std::less<std::string>,Alloc< std::pair<const std::string,VECTOR_CHAR_TYPE > > > VECTOR_CHAR_IN_MAP_TYPE;
    typedef std::map<std::string, VECTOR_CHAR_IN_MAP_TYPE, std::less<std::string>,Alloc< std::pair<const std::string,VECTOR_CHAR_IN_MAP_TYPE > > >   WUP_DATA_TYPE;

public:
	/**
     * 构造函数
     */
    UniAttribute()
    {
	    _iVer = 3;
    }

    void setVersion(short iVer)
    {
	    _iVer = iVer;
    }
    /**
     * 添加属性值
     * 
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:   属性值
     */
    template<typename T> void put(const std::string& name, const T& t)
    {
        os.reset();

        os.write(t, 0);

		VECTOR_CHAR_TYPE & v = _data[name];
        
        os.swap(v);
		// v.assign(os.getBuffer(), os.getBuffer() + os.getLength());

    }

    void putUnknown(const std::string& name, const std::string& value)
    {
        os.reset();
        os.writeUnknownV2(value);

        VECTOR_CHAR_TYPE & v = _data[name];
        os.swap(v);
        // v.assign(os.getBuffer(), os.getBuffer() + os.getLength());
    }

    void getUnknown(const std::string& name, std::string& value)
    {
        typename VECTOR_CHAR_IN_MAP_TYPE::iterator mit;

        mit = _data.find(name);

        if (mit != _data.end() && mit->second.size()>2)
        {
            //去掉DataHead::eStructBegin,DataHead::eStructEnd
            value = std::string(&mit->second[0]+1, mit->second.size()-2);
            return;

        }
        throw std::runtime_error(std::string("UniAttribute not found key:") +  name);
    }

    /**
     * 获取属性值，属性不存在则抛出异常
     * 
     * @throw std::runtime_error
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:   属性值输出参数
     */
    template<typename T> void get(const std::string& name, T& t)
    {
        typename VECTOR_CHAR_IN_MAP_TYPE::iterator mit;

        mit = _data.find(name);

        if (mit != _data.end())
        {
	        is.reset();

	        is.setBuffer(mit->second);

	        is.read(t, 0, true);

	        return;

        }
        throw std::runtime_error(std::string("UniAttribute not found key:") +  name);
    }
    /**
     * 获取属性值，属性不存在则抛出异常
     * 
     * @throw std::runtime_error
     * @param T:   属性类型
     * @param name:属性名称
     * @return T:  属性值
     */
    template<typename T> T get(const std::string& name)
    {
        T t;

        get<T>(name, t);

        return t;
    }
    /**
     * 获取属性值，忽略异常，不存在的属性返回缺省值
     * 
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:  属性值输出参数
     * @param def:     默认值
     */
    template<typename T> void getByDefault(const std::string& name, T& t, const T& def)
    {
        try
        {
            get<T>(name, t);
        }
        catch (std::runtime_error& e)
        {
            t = def;
        }
    }
    /**
     * 获取属性值(忽略异常，def为缺省值)
     * 
     * @param T:   属性类型
     * @param name:属性名称
     * @param:     默认值
     * @return T:  属性值
     */
    template<typename T> T getByDefault(const std::string& name, const T& def)
    {
        T t;

        getByDefault<T>(name, t, def);

        return t;
    }

    /**
     *清除全部属性值
     */
    void clear() 
    { 
        _data.clear();
    }

    /** 编码
     * 
     * @param buff： 编码结果输出参数
     */
    void encode(std::string& buff)
    {
        os.reset();

        os.write(_data, 0);

        os.swap(buff);	
        // buff.assign(os.getBuffer(), os.getLength());
    }

    /** 编码
     * 
     * @param buff： 编码结果输出参数
     */
    void encode(std::vector<char>& buff)
    {
        os.reset();

        os.write(_data, 0);

        os.swap(buff);
        // buff.assign(os.getBuffer(), os.getBuffer() + os.getLength());
    }

    /** 编码
     * 
     * @throw std::runtime_error
     * @param buff：输出存放编码结果的buffer指针
     * @param len： 输入buff长度，输出编码结果长度
     */
    void encode(char* buff, size_t & len)
    {   
        os.reset();

        os.write(_data, 0);

        if(len < os.getLength()) throw std::runtime_error("encode error, buffer length too short");
        memcpy(buff, os.getBuffer(), os.getLength());
        len =  os.getLength();
    }

    /** 解码
     * 
     * @throw std::runtime_error
     * @param buff：待解码字节流的buffer指针
     * @param len： 待解码字节流的长度
     */
    void decode(const char* buff, size_t len)
    {
        is.reset();

        is.setBuffer(buff, len);

        _data.clear();
	
        is.read(_data, 0, true);
		
    }
    /**
     * 解码
     * 
     * @throw std::runtime_error
     * @param buff： 待解码的字节流
     */
    void decode(const std::vector<char>& buff)
    {
        is.reset();

        is.setBuffer(buff);
	
        _data.clear();
	
        is.read(_data, 0, true);
		
    }

    /**
     * 获取已有的属性
     * 
     * @return const std::map<std::string,map<std::string,vector<char>>>& : 属性map
     */
    const std::map<std::string, std::vector<char> >& getData() const
    {
	    return _data;
    }

    /**
     * 判断属性集合是否为空
     * 
     * @return bool:属性是否为空
     */
    bool isEmpty()
    {
        return _data.empty();
    }

    /**
     * 获取属性集合大小
     * 
     * @return size_t:  集合大小
     */
    size_t size()
    {
        return _data.size();
    }

    /**
     * 判断属性是否存在
     * 
     * @param key:属性名称
     * @return bool:是否存在
     */
    bool containsKey(const std::string & key)
    {
        return _data.find(key) != _data.end();
    }

protected:
    VECTOR_CHAR_IN_MAP_TYPE _data;
    short _iVer;

public:
    tars::TarsInputStream<TReader>     is;
    tars::TarsOutputStream<TWriter>    os;
};

/////////////////////////////////////////////////////////////////////////////////
// 请求、回应包封装类

template<typename TWriter = tars::BufferWriter, typename TReader = tars::BufferWriter,template<typename> class Alloc = std::allocator >
struct UniPacket : protected  tars::RequestPacket, public UniAttribute<TWriter, TReader, Alloc>
{
public:
    /**
     * 构造函数
     */
    UniPacket() 
    {
        iVersion = 3; cPacketType = 0; 

        iMessageType = 0; iRequestId = 0; 

        sServantName = ""; sFuncName = ""; 

        iTimeout = 0; sBuffer.clear(); 

        context.clear(); status.clear(); 

        UniAttribute<TWriter, TReader,Alloc>::_iVer = iVersion;

        UniAttribute<TWriter, TReader,Alloc>::_data.clear();

        UniAttribute<TWriter, TReader,Alloc>::_data.clear();
    }

    /**
     * 拷贝构造
     * @param tup
     */
    UniPacket(const UniPacket &tup)  { *this = tup;}

    void setVersion(short iVer)
    {
        UniAttribute<TWriter, TReader,Alloc>::_iVer = iVer;
        iVersion = iVer;
    }

    /**
     * 由请求包生成回应包基本结构，回填关键的请求信息
     * 
     * @return UniPacket： 回应包
     */
    UniPacket createResponse()
    {
        UniPacket respPacket;

        respPacket.sServantName = sServantName;
        respPacket.sFuncName    = sFuncName;
        respPacket.iRequestId   = iRequestId;

        return respPacket;
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * 
     * @throw std::runtime_error
     * @param buff： 编码结果输出参数
     */
    void encode(std::string& buff)
    {
        encodeBuff<std::string>(buff);
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * 
     * @throw std::runtime_error
     * @param buff： 编码结果输出参数
     */
    void encode(std::vector<char>& buff)
    {
        encodeBuff<std::vector<char>>(buff);
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * @throw std::runtime_error
     * @param buff：存放编码结果的buffer指针
     * @param len： 输入buff长度，输出编码结果长度
     */
    void encode(char* buff, size_t & len)
    {
        tars::TarsOutputStream<TWriter>& os = UniAttribute<TWriter, TReader>::os;

        os.reset();

        doEncode(os);

        os.reset();
 
        writeTo(os);

        tars::Int32 iHeaderLen = htonl(sizeof(tars::Int32) + os.getLength());
        if (len < sizeof(tars::Int32) + os.getLength()) throw std::runtime_error("encode error, buffer length too short");

        memcpy(buff, &iHeaderLen, sizeof(tars::Int32));
        memcpy(buff + sizeof(tars::Int32), os.getBuffer(), os.getLength());

        len = sizeof(tars::Int32) + os.getLength();
    }

    /** 解码
     * 
     * @throw std::runtime_error
     * @param buff：待解码字节流的buffer指针
     * @param len： 待解码字节流的长度
     */

    void decode(const char* buff, size_t len)
    {
        if(len < sizeof(tars::Int32)) throw std::runtime_error("packet length too short, first 4 bytes must be buffer length.");
    
        tars::TarsInputStream<TReader> &is = UniAttribute<TWriter, TReader,Alloc>::is;

        is.reset();

        is.setBuffer(buff + sizeof(tars::Int32), len - sizeof(tars::Int32));

        readFrom(is);

        UniAttribute<TWriter, TReader,Alloc>::_iVer = iVersion;

        is.reset();

        is.setBuffer(sBuffer);

        UniAttribute<TWriter, TReader,Alloc>::_data.clear();
	
        is.read(UniAttribute<TWriter, TReader,Alloc>::_data, 0, true);
		
    }
public:
    /**
     * 获取消息version
     * @return tars::Short
     */
    tars::Short getVersion() const { return iVersion; }
    /**
     * 获取消息ID
     * @return tars::Int32
     */
    tars::Int32 getRequestId() const { return iRequestId; }
    /**
     * 设置请求ID
     * @param value
     */
    void setRequestId(tars::Int32 value) { iRequestId = value; }
    /**
     * 获取对象名称
     * @return const std::string&
     */
    const std::string& getServantName() const { return sServantName; }
    /**
     * 设置对象名称
     * @param value
     */
    void setServantName(const std::string& value) { sServantName = value; }
    /**
     * 获取方法名
     * @return const std::string&
     */
    const std::string& getFuncName() const { return sFuncName; }
    /**
     * 设置方法名
     * @param value
     */
    void setFuncName(const std::string& value) { sFuncName = value; }

protected:
    template<typename T>
    void encodeBuff(T& buff)
    {
        tars::TarsOutputStream<TWriter>& os = UniAttribute<TWriter, TReader>::os;

        os.reset();

        doEncode(os);

        os.reset();
 
        tars::Int32 iHeaderLen = 0;

        //	先预留4个字节长度
	    os.writeBuf((const char *)&iHeaderLen, sizeof(iHeaderLen));
        
        writeTo(os);

	    os.swap(buff);

	    assert(buff.size() >= 4);

	    iHeaderLen = htonl((int)(buff.size()));

	    memcpy(&buff[0], (const char *)&iHeaderLen, sizeof(iHeaderLen));
    }

    /**
     * 内部编码
     */
    void doEncode(tars::TarsOutputStream<TWriter>& os)
    {
        //ServantName、FuncName不能为空
        if (sServantName.empty()) throw std::runtime_error("ServantName must not be empty");
        if (sFuncName.empty())    throw std::runtime_error("FuncName must not be empty");

        os.reset();

        os.write(UniAttribute<TWriter, TReader>::_data, 0);

        os.swap(sBuffer);

        os.reset();
    }
};

/////////////////////////////////////////////////////////////////////////////////
// 调用TARS的服务时使用的类

template<typename TWriter = tars::BufferWriter, typename TReader = tars::BufferWriter,template<typename> class Alloc = std::allocator>
struct TarsUniPacket: public UniPacket<TWriter, TReader,Alloc>
{
public:
    TarsUniPacket(){};
    TarsUniPacket(const UniPacket<TWriter, TReader,Alloc> &tup) 
    : UniPacket<TWriter, TReader,Alloc>(tup) {};

    /**
     * 设置协议版本
     * @param value
     */
    void setTarsVersion(tars::Short value) { UniPacket<TWriter, TReader,Alloc>::setVersion(value); }

    /**
     * 设置调用类型
     * @param value
     */
    void setTarsPacketType(tars::Char value) { this->cPacketType = value; }

    /**
     * 设置消息类型
     * @param value
     */
    void setTarsMessageType(tars::Int32 value) { this->iMessageType = value; }

    /**
     * 设置超时时间
     * @param value
     */
    void setTarsTimeout(tars::Int32 value) { this->iTimeout = value; }

    /**
     * 设置参数编码内容
     * @param value
     */
    void setTarsBuffer(const std::vector<tars::Char>& value) { this->sBuffer = value; }

    /**
     * 设置上下文
     * @param value
     */
    void setTarsContext(const std::map<std::string, std::string>& value) { this->context = value; }

    /**
     * 设置特殊消息的状态值
     * @param value
     */
    void setTarsStatus(const std::map<std::string, std::string>& value) { this->status = value; }

    /**
     * 获取协议版本
     * @return tars::Short
     */
    tars::Short getTarsVersion() const { return this->iVersion; }

    /**
     * 获取调用类型
     * @return tars::Char
     */
    tars::Char getTarsPacketType() const { return this->cPacketType; }

    /**
     * 获取消息类型
     * @return tars::Int32
     */
    tars::Int32 getTarsMessageType() const { return this->iMessageType; }

    /**
     * 获取超时时间
     * @return tars::Int32
     */
    tars::Int32 getTarsTimeout() const { return this->iTimeout; }

    /**
     * 获取参数编码后内容
     * @return const std::vector<tars::Char>&
     */
    const std::vector<tars::Char>& getTarsBuffer() const { return this->sBuffer; }

    /**
     * 获取上下文信息
     * @return const std::map<std::string,std::string>&
     */
    const std::map<std::string, std::string>& getTarsContext() const { return this->context; }

    /**
     * 获取特殊消息的状态值
     * @return const std::map<std::string,std::string>&
     */
    const std::map<std::string, std::string>& getTarsStatus() const { return this->status; }

    /**
     * 获取调用tars的返回值
     * 
     * @retrun tars::Int32
     */
    tars::Int32 getTarsResultCode() const
    {
        std::map<std::string, std::string>::const_iterator it;
        if((it = this->status.find(STATUS_RESULT_CODE)) == this->status.end())
        {
            return 0;
        }
        else
        {
            return atoi(it->second.c_str());
        }
    }

    /**
     * 获取调用tars的返回描述
     * 
     * @retrun std::string
     */
    std::string getTarsResultDesc() const
    {
        std::map<std::string, std::string>::const_iterator it;
        if((it = this->status.find(STATUS_RESULT_DESC)) == this->status.end())
        {
            return "";
        }
        else
        {
            return it->second;
        }
    }

};

// #ifdef __GNUC__
// #   if __GNUC__ >3 || __GNUC_MINOR__ > 3
//         typedef UniAttribute<tars::BufferWriter,tars::BufferWriter, __gnu_cxx::__pool_alloc> UniAttrPoolAlloc;
//         typedef UniPacket<tars::BufferWriter,tars::BufferWriter, __gnu_cxx::__pool_alloc> UniPacketPoolAlloc;
//         typedef TarsUniPacket<tars::BufferWriter,tars::BufferWriter, __gnu_cxx::__pool_alloc> TarsUniPacketPoolAlloc;        
// #   endif
// #endif

}
////////////////////////////////////////////////////////////////////////////////////////////////
#endif
