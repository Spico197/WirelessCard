from django.shortcuts import render
# import mysqldb
import simplejson
from django.http import HttpResponse, JsonResponse
from django.core.paginator import Paginator
from django.views.decorators import csrf
import pymongo
import time

"""
device table example:

    *-----------*-------------*
    | device_id | description |
    *-----------*-------------*
    | 123XX123  | device1     |
    *-----------*-------------*

user table example:

    *----------*------------*----------* 
    | uid      | department | password |
    *----------*------------*----------*
    | FFFFFFFF | common     | None     |
    *-----------------------*----------*
    | 00000000 | admin      | 123256   |
    *-----------------------*----------*
    
user data table example:

    *----------*------------*------------*---------------------*------------*-------------------------*----------*
    | uid      | department | device_id  | time                | action_num | description             | toUser   |
    *----------*------------*------------*---------------------*------------*-------------------------*----------*
    | FFFFFFFF | common     | 123XX123   | 2018-05-15 23:17:20 | 0          | user_data               |          |
    *-----------------------*------------*---------------------*------------*-------------------------*----------*
    | 00000000 | admin      | 123XX123   | 2018-05-15 23:19:50 | 3          | delete_user_remain_data | FFFFFFFF |
    *-----------------------*------------*---------------------*------------*-------------------------*----------*
"""
client = pymongo.MongoClient('localhost', 27017)
edc2018 = client['edc2018']
device_table = edc2018['device']
user_table = edc2018['user']
user_data_table = edc2018['user_data']


def current_time():
    return time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))

def uid_check(string):
    flag = True
    if isinstance(string, str):
        if len(string) == 8:
            for character in string:
                if character not in "0123456789ABCDEFabcdef":
                    flag = False
                    break
        else:
            flag = False
    else:
        flag = False
    return flag


def post_data_process(request_POST):
    # print(request_POST)
    post_data = request_POST.decode()
    # print(post_data)
    post_data = post_data.replace('\r', '')
    post_data = post_data.replace('\n', '')
    post_data = post_data.replace(' ', '')
    post_data = post_data.replace(',}', '}')
    # print(post_data)
    post_data = simplejson.loads(post_data)
    return post_data


def post_data_check(data):
    """
    demo: 
    {
        "device_id": "123XX123", // 设备号
        "uid": "AADAD123", // 读取的卡号
        "action": 0, // 读到的操作码，为0-4区间内的一个整数
        "admin": {  // 若进入管理员模式，则需输入此处内容；若非管理员模式，则此处内容仍需输入，但内容可为空
            "admin_uid": "AAAAAAAA", // 管理员UID卡号
            "admin_pswd": "123456"  // 管理员密码
        }
    }
    """
    flag = False
    # 首先判断结构是否完整
    if 'device_id' in data and 'uid' in data and 'action' in data and 'admin' in data:
        # 再判断device_id设备号的正误
        if device_table.find({"device_id": data['device_id']}).count():
            # 再判断action模式
            if 0 <= data['action'] <= 4:
                if data['action'] == 0:
                    if user_table.find({"uid": data['uid'], "department": "common"}).count(): # 普通用户数据上传模式
                        flag = True
                    else:
                        # 用户不存在
                        flag = 2
                if data['action'] == 1: # 创建管理员模式
                    if uid_check(data['uid']) and data['uid'] == data['admin']['admin_uid'] and len(data['admin']['admin_pswd']) == 6:
                        if user_table.find({"uid": data['uid'], "department": "admin"}).count() == 0:
                            flag = True
                        else:   # 管理员账号已存在
                            flag = 9
                    else:
                        # 管理员uid格式错误或管理员uid前后不等或管理员设置的密码有误
                        flag = 3
                if data['action'] == 2: # 创建普通用户模式
                    if uid_check(data['uid']) and \
                        user_table.find({"uid": data['admin']['admin_uid'], "department": "admin", "password": data['admin']['admin_pswd']}).count():
                        if user_table.find({"uid": data['uid'], "department": "common"}).count() == 0:
                            flag = True
                        else:   # 该普通用户已存在
                            flag = 10
                    else:
                        # uid错误或管理员账号或密码错误
                        flag = 4
                if 3 <= data['action'] <= 4: # 管理员操作模式
                    if uid_check(data['uid']) and \
                        user_table.find({"uid": data['admin']['admin_uid'], "department": "admin", "password": data['admin']['admin_pswd']}).count():
                        flag = True
                    else:
                        # uid错误或管理员账号或密码错误
                        flag = 4
            else:
                # action错误
                flag = 5
        else:
            # 设备号错误
            flag = 6
    else:
        # 数据结构不完整
        flag = 7
    return flag

def decodeString(string):
    re = ""
    for ch in list(string):
        re += chr(ord(ch)-1)
    return re

def data_post(request):
    response = {
        'error': 0,
        'uid': None,
    }
    if request.method == 'POST':
        # print(request)
        # print(request.body)
        try:
            try:
                print(request.body)
                post_data = post_data_process(request.body) # convert request data to dict (json)
                if post_data['uid'] != "":
                    post_data['uid'] = post_data['uid'].upper()
                    post_data['uid'] = decodeString(post_data['uid'])
                if post_data['admin']['admin_uid'] != "":
                    post_data['admin']['admin_uid'] = post_data['admin']['admin_uid'].upper()
                    post_data['admin']['admin_uid'] = decodeString(post_data['admin']['admin_uid'])
                if post_data['admin']['admin_pswd'] != "":
                    post_data['admin']['admin_pswd'] = post_data['admin']['admin_pswd'].upper() 
                    post_data['admin']['admin_pswd'] = decodeString(post_data['admin']['admin_pswd'])
                print(post_data)
                # print(decodeString(post_data['uid']))
            except:
                # 数据处理失败
                response['error'] = 1
                raise Exception
            if post_data_check(post_data) is True:
                response['uid'] = post_data["uid"]
                if post_data['action'] == 0:
                    # 普通数据插入
                    user_data_table.insert_one({
                        "uid": post_data['uid'],
                        "department": "common",
                        "time": current_time(),
                        "device_id": post_data['device_id'],
                        "action_num": 0,
                        "description": "user_data",
                        "toUser": None
                    })
                    response['error'] = 0

                elif post_data['action'] == 1:
                    # 新建管理员
                    user_table.insert_one({
                        "uid": post_data['admin']['admin_uid'],
                        "department": "admin",
                        "password": post_data['admin']['admin_pswd']
                    })
                    user_data_table.insert_one({
                        "uid": post_data['admin']['admin_uid'],
                        "department": "admin",
                        "time": current_time(),
                        "device_id": post_data['device_id'],
                        "action_num": 1,
                        "description": "new_admin",
                        "toUser": None
                    })
                    response['error'] = 0

                elif post_data['action'] == 2:
                    # 新建普通用户
                    user_table.insert_one({
                        "uid": post_data['uid'],
                        "department": "common",
                        "password": None
                    })
                    user_data_table.insert_one({
                        "uid": post_data['admin']['admin_uid'],
                        "department": "admin",
                        "time": current_time(),
                        "device_id": post_data['device_id'],
                        "action_num": 2,
                        "description": "new_user",
                        "toUser": post_data["uid"]
                    })
                    response['error'] = 0

                elif post_data['action'] == 3:
                    # 删除用户但保留数据
                    user_table.delete_many({
                        "uid": post_data["uid"],
                        "department": "common"
                    })
                    user_data_table.insert_one({
                        "uid": post_data['admin']['admin_uid'],
                        "department": "admin",
                        "time": current_time(),
                        "device_id": post_data['device_id'],
                        "action_num": 3,
                        "description": "delete_user_remain_data",
                        "toUser": post_data["uid"]
                    })
                    response['error'] = 0

                elif post_data['action'] == 4:
                    # 删除用户及其数据
                    user_table.delete_many({
                        "uid": post_data["uid"],
                        "department": "common"
                    })
                    user_data_table.delete_many({
                        "uid": post_data["uid"],
                        "department": "common"
                    })
                    user_data_table.insert_one({
                        "uid": post_data['admin']['admin_uid'],
                        "department": "admin",
                        "time": current_time(),
                        "device_id": post_data['device_id'],
                        "action_num": 4,
                        "description": "delete_user_without_data",
                        "toUser": post_data["uid"]
                    })
                    response['error'] = 0
                else:
                    response['error'] = 7
                    raise Exception
            else:
                response['error'] = post_data_check(post_data)
                raise Exception
        except:
            pass
    else:
        # 不是POST请求
        response['error'] = 8
    
    del response['uid']
    res = JsonResponse(response)
    del res['Date']
    del res['Server']
    del res['X-Frame-Options']
    print("error: ", response['error'])
    # return HttpResponse(simplejson.dumps(response), content_type='application/json')
    return res


def console(request):
    context = {
        'device_number': device_table.count(),
        'user_number': user_table.count(),
        'user_data_number': user_data_table.count(),
    }
    if request.POST:
        device_id = request.POST['device_id']
        uid = request.POST['uid']
        if device_id != "":
            if device_table.find({"device_id": device_id}).count() != 0:
                if user_data_table.find({"device_id": device_id}).count != 0:
                    context['device_data'] = user_data_table.find({"device_id": device_id})
                    
        if uid != "":
            if user_table.find({"uid": uid}).count() != 0:
                if user_data_table.find({"uid": uid}).count != 0:
                    context['user_data'] = user_data_table.find({"uid": uid})
    return render(request, "console.html", context=context)

def console_user(request):
    limit = 10
    paginator = Paginator(list(user_table.find()), limit)
    page = request.GET.get('page', 1)
    loaded = paginator.page(page)

    context = {
        'users': user_table.find(),
        'device_number': device_table.count(),
        'user_number': user_table.count(),
        'user_data_number': user_data_table.count(),
        'ArticleInfo': loaded,
    }
    return render(request, 'user.html', context=context)

def console_user_data(request):
    limit = 10
    paginator = Paginator(list( user_data_table.find().sort([("time", -1)]) ), limit)
    page = request.GET.get('page', 1)
    loaded = paginator.page(page)

    context = {
        'device_number': device_table.count(),
        'user_number': user_table.count(),
        'user_data_number': user_data_table.count(),
        'ArticleInfo': loaded,
    }
    return render(request, 'user_data.html', context=context)

def console_device_register(request):
    flag = False
    if request.POST:
        data = {}
        data['device_id'] = request.POST['device_id']
        data['description'] = request.POST['description']
        if device_table.find({"device_id": data['device_id']}).count() == 0:
            device_table.insert_one(data)
            if device_table.find({"device_id": data['device_id']}).count():
                flag = True
    if flag:
        context = {
            'device_number': device_table.count(),
            'user_number': user_table.count(),
            'user_data_number': user_data_table.count(),
            'ok': "注册成功",
        }
        return render(request, "device_register.html", context=context)
    else:
        context = {
            'device_number': device_table.count(),
            'user_number': user_table.count(),
            'user_data_number': user_data_table.count(),
        }
        return render(request, "device_register.html", context=context)

def console_user_register(request):
    flag = False
    if request.POST:
        data = {}
        data['uid'] = request.POST['uid']
        data['department'] = request.POST['department']
        data['password'] = request.POST['password']
        if user_table.find({"uid": data['uid']}).count() == 0 and data['department'] in ["admin", "common"] and uid_check(data['uid']):
            if data['department'] == "admin" and len(data['password']) == 6:
                user_table.insert_one(data)
            elif data['department'] == "common":
                user_table.insert_one(data)
            if user_table.find({"uid": data['uid']}).count():
                flag = True
    if flag:
        context = {
            'device_number': device_table.count(),
            'user_number': user_table.count(),
            'user_data_number': user_data_table.count(),
            'ok': "注册成功",
        }
        return render(request, "user_register.html", context=context)
    else:
        context = {
            'device_number': device_table.count(),
            'user_number': user_table.count(),
            'user_data_number': user_data_table.count(),
        }
        return render(request, "user_register.html", context=context)

def console_device(request):
    limit = 10
    paginator = Paginator(list(device_table.find()), limit)
    page = request.GET.get('page', 1)
    loaded = paginator.page(page)

    context = {
        'users': user_table.find(),
        'device_number': device_table.count(),
        'user_number': user_table.count(),
        'user_data_number': user_data_table.count(),
        'ArticleInfo': loaded,
    }
    return render(request, 'device.html', context=context)
