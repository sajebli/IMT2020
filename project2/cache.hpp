

/*
 
 This class defines the cache decorator used to store the pre-calculated values of each function in a dictionary like type, the function is actually 
 a mapping between arguments and the returned value, so we can use the arguments as keys to store the values in a map (dictionary) with the objects of class X
 as keys, in our case the keys are the <Time stepTime>, and class Y object as values to be stored, Real objects in our case .

*/

#ifndef cache_hpp s
#define cache_hpp 
#include <functional>
#include <map>
template<class X, class Y> 
class Cache{
public :
    
    Cache() { }

    Y operator ()(X key) const {
        auto index = _map.find (key ); 
        if(index != _map.end()){
            return _map[key] ;
        }
        auto result = f(key);
        _map[key] = result;
        return result;
    }
    // this method sets the cached function f with a given function f_
    void setf(const std::function<Y(X)> _f){ 
        f=_f;
        }

private :
    std :: function<Y(X)> f ;
    mutable std::map<X,Y> _map; 
    }; 
#endif