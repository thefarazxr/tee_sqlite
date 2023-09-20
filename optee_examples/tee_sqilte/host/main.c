/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
/* For the UUID (found in the TA's h-file(s)) */
#include <tee_sqlite.h>

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	NotUsed = NULL;

	// for (int i = 0; i < argc; ++i)
	// {
	// printf("%s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
	// }

	// printf("\n");

	return 0;
}

int main(void)
{
	int fd = 0;
	time_t t1 = time(0);
	sqlite3 *db = NULL;
	int rc = sqlite3_open("/tmp/test2.db", &db);

	if (rc != SQLITE_OK)
	{
		printf("Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}

	char *err_msg = NULL;

	const char *sql = "DROP TABLE IF EXISTS Cars;"
					  "CREATE TABLE Cars(Id INT, Name TEXT, Price INT);";
	//   "INSERT INTO Cars VALUES(2, 'Mercedes', 57127);";
	//   "INSERT INTO Cars VALUES(3, 'Skoda', 9000);"
	//   "INSERT INTO Cars VALUES(4, 'Volvo', 29000);"
	//   "INSERT INTO Cars VALUES(5, 'Bentley', 350000);"
	//   "INSERT INTO Cars VALUES(6, 'Citroen', 21000);"
	//   "INSERT INTO Cars VALUES(7, 'Hummer', 41400);"
	//   "INSERT INTO Cars VALUES(8, 'Volkswagen', 21600);";

	rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
	if (rc != SQLITE_OK)
	{

		printf("SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);

		return 1;
	}

	// sql = "INSERT INTO Cars VALUES(3, 'Skoda', 9000);";
	// for (int i = 0; i < 1000; i++)
	// {
	// 	rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
	// 	if (rc != SQLITE_OK)
	// 	{
	// 		printf("SQL error: %s\n", err_msg);
	// 		sqlite3_free(err_msg);
	// 		sqlite3_close(db);

	// 		return 1;
	// 	}
	// }
	sqlite3_close(db);
	time_t t2 = time(0);
	printf("normal insert: %ld\n", t2 - t1);

	t1 = time(0);
	rc = sqlite3_open("/tmp/test2.db", &db);
	sql = "select * from Cars;";
	for (int i = 0; i < 1000; i++)
	{
		rc = sqlite3_exec(db, sql, NULL, callback, &err_msg);
		if (rc != SQLITE_OK)
		{
			printf("SQL error: %s\n", err_msg);
			sqlite3_free(err_msg);
			sqlite3_close(db);

			return 1;
		}
	}
	sqlite3_close(db);
	t2 = time(0);
	printf("normal select: %ld\n", t2 - t1);

	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_TEE_SQLITE_UUID;
	uint32_t err_origin;
	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
						   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			 res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
									 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = 42;

	/*
	 * TA_HELLO_WORLD_CMD_INC_VALUE is the actual function in the TA to be
	 * called.
	 */
	printf("Invoking TA to increment %d\n", op.params[0].value.a);
	res = TEEC_InvokeCommand(&sess, TA_TEE_SQLITE_EXEC, &op,
							 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			 res, err_origin);
	printf("TA incremented value to %d\n", op.params[0].value.a);

	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);
	return 0;
}
