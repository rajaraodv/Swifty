//
//  SFNetworkEngine.h
//  NetworkSDK
//
//  Created by Qingqing Liu on 9/24/12.
//  Copyright (c) 2012 salesforce.com. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SFNetworkOperation.h"
#import "SFNetworkCoordinator.h"

// Salesforce's wrapper around common Reachability NetworkStatus Compatible Names.
typedef enum {
	SFNotReachable     = 0,
	SFReachableViaWiFi = 2,
	SFReachableViaWWAN = 1
} SFNetworkStatus;

extern NSString * const SFNetworkOperationGetMethod;
extern NSString * const SFNetworkOperationPostMethod;
extern NSString * const SFNetworkOperationPutMethod;
extern NSString * const SFNetworkOperationDeleteMethod;
extern NSString * const SFNetworkOperationPatchMethod;
extern NSString * const SFNetworkOperationHeadMethod;

/** Notification that will be posted when SFNetworkEngine detects network change
 
 When posted, `SFNetworkStatus` will wraped in NSNumber as the `[notification object]
 */
extern NSString * const SFNetworkOperationReachabilityChangedNotification;

/** Notification that will be posted when SFNetworkEngine cancels all operations
 */
extern NSString * const SFNetworkOperationEngineOperationCancelledNotification;

/** Notification that will be posted when SFNetworkEngine suspends all pending operations
 */
extern NSString * const SFNetworkOperationEngineSuspendedNotification;

/** Notification that will be posted when SFNetworkEngine starts to resume all operations
 */
extern NSString * const SFNetworkOperationEngineResumedNotification;



@class SFNetworkEngine;

/** Prototol to implement to handle session refresh 
 
*/
@protocol SFNetworkEngineDelegate <NSObject>
@required

/** Implement this method to refresh session 
 
 When session is refreshed, `SFNetworkEngineDelegate` should call `setCoordinator` to update access information
 */
- (void)refreshSessionForNetworkEngine:(SFNetworkEngine *)networkEngine;

@end

/**
 Main class used to manage and send `SFNetworkOperation`
 
 Caller of SFNetworkEngine should call `sharedInstance` to initalize the SFNetworkEngine when OAuth is completed successfully and set `coordinator`.
 
 SFNetworkEngine will perform the following task by default
 - Detect duplication request and associate callback blocks for the duplicate operation to the existing operation
 - Monitor network change and publish  a `SFNetworkOperationReachabilityChangedNotification` notification will be posted when reachability changed with `SFNetworkStatus` wraped in NSNumber as the `[notification object]`
 - Manange network concurrence based on network type
 - Automatically start background handling for running operation
 - Suspend all pending operations when app enters background and resumes them when app becomes active. Set `suspendRequestsWhenAppEntersBackground` to change this behavior
 - Encrypt downloaded content that will be stored as a local file. Change `[SFNetworkOperation encryptDownloadedFile]` to change this behavior
 */
@interface SFNetworkEngine : NSObject

/** `​SFNetwork​Engine` relies on `SFNetworkCoordinator` to know where to connect and who to connect as and with what access token
 
 `SFNetworkEngineDelegate` should call this method when access token is refreshed
 */
@property (nonatomic, strong) SFNetworkCoordinator *coordinator;

/** If plan to use `SFNetworkEngine` against a non-SFDC source, use remote host to initialize the `SFNetworkEngine` instead.
 
    When initialized this way without setting coordinator, requiresAccessToken will be set 
    to NO for `SFNetworkOperation`. Caller is responsible for using `customHeaders` to set necessary authorization header if needed
*/
@property (nonatomic, copy) NSString *remoteHost;

/** Custom HTTP headers that will be set for all `SFNetworkOperation` before executing
 
 `SFNetworkEngine` will automatically set the following headers if customHeaders is nil or do not contain the specific header key
 - Authorization header with `[[SFOAuthCoordinator credentials] accessToken]`
 - User-Agent header with application name, version and OS information
 */
@property (nonatomic, copy) NSDictionary *customHeaders;

/** Handler that you implement to monitor reachability changes
 
 if `reachabilityChangedHandler` is not set, caller of SFNetworkEngine can also observe `SFNetworkOperationReachabilityChangedNotification` notification. `SFNetworkEngine` will be posted when reachability changed with `NetworkStatus` wrapped in NSNumber as the `[notification object]`
 */
@property (nonatomic, copy) void (^reachabilityChangedHandler)(SFNetworkStatus ns);


/** Return current network status*/
@property (nonatomic, assign, readonly) SFNetworkStatus networkStatus;

/**Default time out for all `SFNetworkOperation` executed by `SFNetworkEngine` in seconds. Default is 180 seconds
 */
@property (nonatomic, assign) NSTimeInterval operationTimeout;

/** Set to YES if enable HTTP Pipeling for HTTP GET requests. Default value is YES
 */
@property (nonatomic, assign) BOOL enableHttpPipeling;

/** Set to YES to allow `SFNetworkEngine` to let `SFNetworkOperation` to use local test file to simulate server response if `[SFNetworkOperation localTestDataPath]` is set. Default value is NO
 
 When this property is set to YES, you can set `[SFNetworkOperation localTestDataPath]` with the full path to a 
 local test file, and when `[SFNetworkEngine enqueueOperation]` is called, the operation
 will read response data from the specified test file instead of making a remote call and invoke completion blocks
 following the normal flow
 
 Make sure you set this property to NO in release build
*/
@property (nonatomic, assign) BOOL supportLocalTestData;

/**Set to true to suspend all pending requests when app enters background. Default is YES*/
@property (nonatomic, assign, getter = shouldSuspendRequestsWhenAppEntersBackground) BOOL suspendRequestsWhenAppEntersBackground;


/** NetworkEngine delegate responsible for refresh session */
@property (weak) id<SFNetworkEngineDelegate> delegate;
 
/** Returns the singleton instance of `SFNetworkEngine`
 * After a successful oauth login with an SFOAuthCoordinator, you
 * should set the coordinator property of this instance.
 */
+ (SFNetworkEngine *)sharedInstance;

/** Set value for the specified HTTP header
 
@param value Header value.
@param key Header key
*/
- (void)setHeaderValue:(NSString *)value forKey:(NSString *)key;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call
 *
 * @param url Url to the remote service to invoke. If this url is a relative URL, `SFNetworkEngine` will automatically construct the full URL.
 * @param params Key & value pair as request parameters
 * @param method the http method to use. Valid value include GET, POST, DELETE, PUT and PATCH
 * @param useSSL Set to YES to use SSL connection
 * @return the `SFNetworkOperation` object that can be executed by calling `enqueueOperation method
 */
- (SFNetworkOperation *)operationWithUrl:(NSString *)url params:(NSDictionary *)params httpMethod:(NSString *)method ssl:(BOOL)useSSL;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote request under SSL.
 *
 * @param url Url to the remote service to invoke.  If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 * @param method the http method to use. Valid value include GET, POST, DELETE, PUT and PATCH
 * @return the `SFNetworkOperation` object that can be executed by calling `enqueueOperation method
 */
- (SFNetworkOperation *)operationWithUrl:(NSString *)url params:(NSDictionary *)params httpMethod:(NSString *)method;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call using `SFNetworkOperationGetMethod` method under SSL

 * @param url Url to the remote service to invoke. If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 */
- (SFNetworkOperation *)get:(NSString *)url params:(NSDictionary *)params;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call using `SFNetworkOperationPostMethod` method under SSL
 
 * @param url Url to the remote service to invoke. If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 */
- (SFNetworkOperation *)post:(NSString *)url params:(NSDictionary *)params;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call using `SFNetworkOperationPutMethod` method under SSL
 
 * @param url Url to the remote service to invoke. If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 */
- (SFNetworkOperation *)put:(NSString *)url params:(NSDictionary *)params;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call using `SFNetworkOperationDeleteMethod` method under SSL
 
 * @param url Url to the remote service to invoke. If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 */
- (SFNetworkOperation *)delete:(NSString *)url params:(NSDictionary *)params;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call using `SFNetworkOperationPathMethod` method under SSL
 
 * @param url Url to the remote service to invoke. If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 */
- (SFNetworkOperation *)patch:(NSString *)url params:(NSDictionary *)params;

/** Returns a `SFNetworkOperation` that can be used to execute the specified remote call using `SFNetworkOperationPathMethod` method under SSL
 
 * @param url Url to the remote service to invoke. If this url is a relative URL, `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to it
 * @param params Key & value pair as request parameters
 */
- (SFNetworkOperation *)head:(NSString *)url params:(NSDictionary *)params;

/**Returns `SFNetworkOperation` for the specified condition
 
 * @param url Url to the remote service to invoke. This url does not start with HTTP protocol (http or https), `[[SFOAuthCoordinator credentials] instanceUrl]` will be automatically added to the url that will be executed
 * @param params Key & value pair as request parameters
 * @param method the http method to use. Valid value include GET, POST, DELETE, PUT and PATCH
 * @return SFNetworkOperation` object if there is a pending or running operation matching the specified url, parameters and HTTP method. If no matching operation object is found in the queue, it will return nil
 */
- (SFNetworkOperation *)activeOperationWithUrl:(NSString *)url params:(NSDictionary *)params httpMethod:(NSString *)method;

/**Enqueues `SFNetworkOperation` for execution
 
 Enqueued operation will be executed by `SFNetworkEngine` based on it's priority and dependencies if any
 @param operation `SFNetworkOperation` object to be enqueued and executed by `SFNetworkEngine`
 */
- (void)enqueueOperation:(SFNetworkOperation*)operation;

/**Clean up the SFNetworkEngine due to host change or logout
 
 This method should be called upon user logout
 */
- (void)cleanup;

/** Returns YES if `[[SFOAuthCoordinator credentials] instanceUrl]` is reachable
 *	If `coordinator` is not set before this method is called, it will return NO
 */
- (BOOL)isReachable;

/** Fatal OAuth error happened. Call error block of all operations stored in `operationsWaitingForAccessToken` queue
 */
- (void)failOperationsWaitingForAccessTokenWithError:(NSError *)error;

/** Cancel all operations that are waiting to be excecuted
 */
- (void)cancelAllOperations;

/** Cancel all operations with a specific tag that are waiting to be excecuted
 
 This method will cancel all operations that are either running or waiting to be executed that matches the specific operation tag
 
  @param operationTag Operation tag
 */
- (void)cancelAllOperationsWithTag:(NSString *)operationTag;

/**Suspend all operations that are waiting to be excecuted
 */
- (void)suspendAllOperations;

/**Resume all operations that are suspended
 */
- (void)resumeAllOperations;

/**Returns YES of there are pending requests matching the specified operation tag
 
 @param operationTag Operation tag
 */
- (BOOL)hasPendingOperationsWithTag:(NSString *)operationTag;

/** Returns an array of operations that matches the tag */
- (NSArray *)operationsWithTag:(NSString *)operationTag;


/** Replay all operations stored in `operationsWaitingForAccessToken` queue
 */
- (void)replayOperationsWaitingForAccessToken;

@end
