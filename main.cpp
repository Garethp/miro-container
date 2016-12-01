#include <phpcpp.h>
#include <iostream>

class Container : public Php::Base, public Php::ArrayAccess
{
private:
    std::map<std::string,Php::Value> _map;
    std::map<std::string,Php::Value> _values;
    bool isCallable(Php::Value item) {
        return item.isCallable() && !item.isScalar();
    }

public:
    Container() = default;

    virtual ~Container() = default;

    Php::Value get(Php::Parameters &parameters)
    {
        return offsetGet(parameters[0]);
    }

    Php::Value has(Php::Parameters &parameters)
    {
        return offsetExists(parameters[0]);
    }

    virtual bool offsetExists(const Php::Value &key) override
    {
        return _map.find(key) != _map.end();
    }

    virtual void offsetSet(const Php::Value &key, const Php::Value &value) override
    {
        if (!isCallable(value)) {
            _values[key] = value;
        }

        _map[key] = value;
    }

    virtual Php::Value offsetGet(const Php::Value &key) override
    {
        if (!offsetExists(key)) {
            throw Php::Exception("Key Not Found");
        }

        if (_values.find(key) == _values.end()) {
            _values[key] = _map[key]();
        }

        return _values[key];
    }

    virtual void offsetUnset(const Php::Value &key) override
    {
        _map.erase(key);
    }
};

/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {
    
    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("miro-container", "1.0");

        //The Interop\Container namespaces
        Php::Interface containerInteropInterface("ContainerInterface");
        containerInteropInterface.method("get", {
               Php::ByVal({"id"})
        });
        containerInteropInterface.method("has", {
                Php::ByVal({"id"})
        });

        Php::Interface notFoundException("NotFoundException");

        Php::Namespace containerInterop("Interop\\Container");
        containerInterop.add(std::move(containerInteropInterface));

        Php::Namespace containerInteropException("Exception");
        containerInteropException.add(std::move(notFoundException));

        containerInterop.add(std::move(containerInteropException));

        extension.add(std::move(containerInterop));

        //The Psr\Container Interface
        Php::Interface psrContainerInterface("ContainerInterface");
        psrContainerInterface.method("get", {
                Php::ByVal({"id"})
        });
        psrContainerInterface.method("has", {
                Php::ByVal({"id"})
        });

        Php::Namespace psrContainer("Psr\\Container");
        psrContainer.add(std::move(psrContainerInterface));

        extension.add(std::move(psrContainer));

        //The Miro\Container Interface
        Php::Namespace miroContainerNamespace("Miro");
        
        Php::Class<Container> container("Container");
        container.implements(containerInteropInterface);
        container.implements(psrContainerInterface);
        container.method<&Container::get> ("get", {
                Php::ByVal({"id"})
        });
        container.method<&Container::has> ("has", {
                Php::ByVal({"id"})
        });

//        Php::Class<NotFoundException> notFoundException("NotFoundException");
//        notFoundException.implements(notFoundException);
//
//        miroContainerNamespace.add(std::move(notFoundException));
        miroContainerNamespace.add(std::move(container));

        extension.add(std::move(miroContainerNamespace));
        
        // return the extension
        return extension;
    }
}
