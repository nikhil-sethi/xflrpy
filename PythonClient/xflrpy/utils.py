def loop_until(condition):
    def inner1(func):    
        def inner2(*args, **kwargs):
            while condition:
                func(*args,**kwargs)
        return inner2
    return inner1