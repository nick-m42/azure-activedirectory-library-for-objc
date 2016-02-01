// Copyright © Microsoft Open Technologies, Inc.
//
// All Rights Reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
// ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A
// PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
//
// See the Apache License, Version 2.0 for the specific language
// governing permissions and limitations under the License.

#import <XCTest/XCTest.h>
#import "ADAuthenticationError.h"
#import "ADAL_Internal.h"

#define ADTAssertContains(_str, _contains) XCTAssertTrue([_str containsString:_contains], "%@ does not contain \"%@\"", _str, _contains)

#define TEST_AUTHORITY @"https://login.windows.net/contoso.com"
#define TEST_REDIRECT_URL [NSURL URLWithString:@"urn:ietf:wg:oauth:2.0:oob"]
#define TEST_RESOURCE @"resource"
#define TEST_USER_ID @"eric_cartman@contoso.com"
#define TEST_CLIENT_ID @"c3c7f5e5-7153-44d4-90e6-329686d48d76"
#define TEST_ACCESS_TOKEN @"access token"
#define TEST_ACCESS_TOKEN_TYPE @"access token type"
#define TEST_REFRESH_TOKEN @"refresh token"
#define TEST_CORRELATION_ID [[NSUUID alloc] initWithUUIDString:@"6fd1f5cd-a94c-4335-889b-6c598e6d8048"]

typedef enum
{
    TEST_LOG_LEVEL,
    TEST_LOG_MESSAGE,
    TEST_LOG_INFO,
    TEST_LOG_CODE,
} ADLogPart;

@class ADTokenCacheItem;
@class ADUserInformation;
@class ADTokenCacheKey;
@class ADTestURLResponse;

@interface XCTestCase (HelperMethods)

- (void)adAssertStringEquals:(NSString *)actual
            stringExpression:(NSString *)expression
                    expected:(NSString *)expected
                        file:(const char *)file
                        line:(int)line;

/*! Used with the class factory methods that create class objects. Verifies
 the expectations when the passed argument is invalid:
 - The creator should return nil.
 - The error should be set accordingly, containing the argument in the description.*/
- (void)adValidateFactoryForInvalidArgument:(NSString *)argument
                             returnedObject:(id)returnedObject
                                      error:(ADAuthenticationError *)error;

- (ADTestURLResponse *)adResponseBadRefreshToken:(NSString *)refreshToken
                                       authority:(NSString *)authority
                                        resource:(NSString *)resource
                                        clientId:(NSString *)clientId
                                   correlationId:(NSUUID *)correlationId;
- (ADTestURLResponse *)adDefaultBadRefreshTokenResponse;

- (ADTestURLResponse *)adDefaultRefreshResponse:(NSString *)newRefreshToken
                                    accessToken:(NSString *)newAccessToken;

/*! Verifies that the correct error is returned when any method was passed invalid arguments.
 */
- (void)adValidateForInvalidArgument:(NSString *)argument
                               error:(ADAuthenticationError *)error;

/*! Sets logging and other infrastructure for a new test.
 The method sets the callback and fails the tests if a the logs contains higher level
 item than the maxLogTolerance. E.g. strict test may set this parameter to ADAL_LOG_LEVEL_INFO,
 so that all warnings and errors will be cause the test to fail.*/
- (void)adTestBegin:(ADAL_LOG_LEVEL)maxLogTolerance;

/*! See description of adTestBegin. */
- (void)adSetLogTolerance: (ADAL_LOG_LEVEL)maxLogTolerance;

/*! Clears logging and other infrastructure after a test */
- (void)adTestEnd;

//The methods help with verifying of the logs:
- (NSString *)adGetLogs:(ADLogPart)logPart;

//Clears all of the test logs. Useful for repeating operations.
- (void)adClearLogs;

- (void)adAssertLogsContain:(NSString *)text
                    logPart:(ADLogPart)logPart
                       file:(const char *)file
                       line:(int)line;

- (void)adAssertLogsDoNotContain:(NSString *)text
                         logPart:(ADLogPart)logPart
                            file:(const char *)file
                            line:(int) line;

//Creates a new item with all of the properties having correct values
- (ADTokenCacheItem *)adCreateCacheItem;
- (ADTokenCacheItem *)adCreateCacheItem:(NSString*)userId;
- (ADTokenCacheItem *)adCreateATCacheItem;
- (ADTokenCacheItem *)adCreateATCacheItem:(NSString *)resource
                                   userId:(NSString *)userId;
- (ADTokenCacheItem *)adCreateMRRTCacheItem;
- (ADTokenCacheItem *)adCreateMRRTCacheItem:(NSString *)userId;
- (ADTokenCacheKey *)adCreateCacheKey;

//Creates a sample user information object
- (ADUserInformation *)adCreateUserInformation:(NSString*)userId;

- (NSString *)adLogLevelLogs;
- (NSString *)adMessagesLogs;
- (NSString *)adInformationLogs;
- (NSString *)adErrorCodesLogs;

//Counts how many times the "contained" is sequentially occurring in "string".
//Example: "bar bar" is contained once in "bar bar bar" and twice in "bar bar bar bar".
- (int)adCountOccurencesOf:(NSString *)contained
                  inString:(NSString *)string;

//The methods help with verifying of the logs:
- (int)adCountOfLogOccurrencesIn:(ADLogPart)logPart
                        ofString:(NSString *)contained;

//Checks if the test coverage is enabled and stores the test coverage, if yes.
- (void)adFlushCodeCoverage;

/* A special helper, which invokes the 'block' parameter in the UI thread and waits for its internal
 callback block to complete.
 IMPORTANT: The internal callback block should end with ASYNCH_COMPLETE macro to signal its completion. Example:
 static volatile int comletion  = 0;
 [self adCallAndWaitWithFile:@"" __FILE__ line:__LINE__ completionSignal: &completion block:^
 {
    [mAuthenticationContext acquireTokenWithResource:mResource
                                     completionBlock:^(ADAuthenticationResult* result)
    {
        //Inner block:
        mResult = result;
        ASYNC_BLOCK_COMPLETE(completion);//Signals the completion
    }];
 }];
 The method executes the block in the UI thread, but runs an internal run loop and thus allows methods which enqueue their
 completion callbacks on the UI thread.
 */
- (void)adCallAndWaitWithFile:(NSString*)file
                         line:(int)line
                    semaphore:(dispatch_semaphore_t)signal
                        block:(void (^)(void)) block;
@end

//Fixes the issue with XCTAssertEqual not comparing int and long values
//Usage: ADAssertLongEquals(5, [self calculateFive]);
#define ADAssertLongEquals(CONST, EXPR) XCTAssertEqual((long)CONST, (long)EXPR)

#define ADAssertThrowsArgument(EXP) \
{ \
    XCTAssertThrowsSpecificNamed((EXP), NSException, NSInvalidArgumentException, "Exception expected for %s", #EXP); \
}

//Usage: ADAssertStringEquals(resultString, "Blah");
#define ADAssertStringEquals(actualParam, expectedParam) \
{ \
    [self adAssertStringEquals:actualParam \
              stringExpression:@"" #actualParam \
                      expected:expectedParam \
                           file:__FILE__ \
                           line:__LINE__]; \
}

//Fixes the problem with the test framework not able to compare dates:
#define ADAssertDateEquals(actualParam, expectedParam) XCTAssertTrue([expectedParam compare:actualParam] == NSOrderedSame)

//Usage: ADAssertLogsContain(TEST_LOG_MESSAGE, "acquireToken");
//       ADAssertLogsContainValue(TEST_LOG_MESSAGE, parameterValue);
// Use ADAssertLogsContain for constant texts and ADAssertLogsContainValue, when passing a string object.
#define ADAssertLogsContain(LOGPART, TEXT) \
{ \
    [self adAssertLogsContain:TO_NSSTRING(TEXT) \
                      logPart:LOGPART \
                         file:__FILE__ \
                         line:__LINE__]; \
}

//"TEXT" should be string object:
#define ADAssertLogsContainValue(LOGPART, TEXT) \
{ \
    [self adAssertLogsContain:TEXT \
                      logPart:LOGPART \
                         file:__FILE__ \
                         line:__LINE__]; \
}

#define ADAssertLogsDoNotContain(LOGPART, TEXT) \
{ \
    [self adAssertLogsDoNotContain:TO_NSSTRING(TEXT) \
                           logPart:LOGPART \
                              file:__FILE__ \
                              line:__LINE__]; \
}

#define ADAssertLogsDoNotContainValue(LOGPART, TEXT) \
{ \
    [self adAssertLogsDoNotContain:TEXT \
                           logPart:LOGPART \
                              file:__FILE__ \
                              line:__LINE__];\
}

//Verifes that "error" local variable is nil. If not prints the error
#define ADAssertNoError XCTAssertNil(error, "Unexpected error occurred: %@", error.errorDetails)
