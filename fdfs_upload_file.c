/**
* Copyright (C) 2008 Happy Fish / YuQing
*
* FastDFS may be copied only under the terms of the GNU General
* Public License V3, which may be found in the FastDFS source kit.
* Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fdfs_client.h"
#include "logger.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


int upload(lua_State *L)
{
	char *conf_filename;
	char *local_filename;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer;
	int result;
	int store_path_index;
	ConnectionInfo storageServer;
	char file_id[128];
        size_t n;	
        char response[255];

        conf_filename = luaL_checklstring(L, 1, &n);
        local_filename = luaL_checklstring(L, 2, &n);
	log_init();
	g_log_context.log_level = LOG_ERR;
	ignore_signal_pipe();
        

	if ((result=fdfs_client_init(conf_filename)) != 0)
	{
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		return errno != 0 ? errno : ECONNREFUSED;
	}

	*group_name = '\0';
	if ((result=tracker_query_storage_store(pTrackerServer, \
	                &storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		return result;
	}

	result = storage_upload_by_filename1(pTrackerServer, \
			&storageServer, store_path_index, \
			local_filename, NULL, \
			NULL, 0, group_name, file_id);
	if (result == 0)
	{
            sprintf(response, "http://%s/%s\n", storageServer.ip_addr, file_id);
            lua_pushstring(L, response);
            return 1;
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}
static luaL_Reg mylibs[] = { 
{"fastdfs_upload", upload},
{NULL, NULL} 
}; 
int luaopen_fastdfs(lua_State* L) 
{
const char* libName = "fastdfs";
luaL_register(L,libName, mylibs);
return 1;
}
//int main() {
 //  upload("/etc/fdfs/client.conf", "/home/yuecaili/Lighthouse.jpg");
  // return 0;
//}

