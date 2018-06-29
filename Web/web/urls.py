"""web URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/1.11/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  url(r'^$', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  url(r'^$', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.conf.urls import url, include
    2. Add a URL to urlpatterns:  url(r'^blog/', include('blog.urls'))
"""
from django.conf.urls import url
from django.contrib import admin
from MainApp.views import *

urlpatterns = [
    url(r'^admin/', admin.site.urls),
    url(r'^console/$', console),
    url(r'^console/user/$', console_user),
    url(r'^console/data/$', console_user_data),
    url(r'^console/device/$', console_device),
    url(r'^console/device_register/$', console_device_register),
    url(r'^console/user_register/$', console_user_register),
    url(r'^data_post/', data_post),
    url(r'/', console),
    url(r'', data_post),
]
