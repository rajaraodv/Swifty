//
//  SFNetworkOperation.h
//  NetworkSDK
//
//  Created by Qingqing Liu on 9/25/12.
//  Copyright (c) 2012 salesforce.com. All rights reserved.
//

#import <Foundation/Foundation.h>
@class SFNetworkOperation;

typedef void (^SFNetworkOperationProgressBlock)(double progress);
typedef void (^SFNetworkOperationCompletionBlock)(SFNetworkOperation* operation);
typedef void (^SFNetworkOperationCancelBlock)(SFNetworkOperation* operation);
typedef void (^SFNetworkOperationErrorBlock)(NSError* error);
typedef NSString* (^SFNetworkOperationEncodingBlock) (NSDictionary* postDataDict);

/** Delegate to implement to get notified on network operation status change
 */
@protocol SFNetworkOperationDelegate <NSObject>

@optional
- (void)networkOperationDidFinish:(SFNetworkOperation *)operation;
- (void)networkOperation:(SFNetworkOperation *)operation didFailWithError:(NSError *)error;
- (void)networkOperationDidCancel:(SFNetworkOperation *)operation;
- (void)networkOperationDidTimeout:(SFNetworkOperation *)operation;
@end


/**
 Main class used to create and execute remote network call
 
 `SFNetworkEngine` should be used to create instance of `SFNetworkOperation`
 */
@interface SFNetworkOperation : NSOperation

/**Custom tag for this operation
 
 Tag can be used to categorize `SFNetworkOperation` and used together with `[SFNetworkEngine hasPendingOperationsWithTag]`
 */
@property (nonatomic, copy) NSString *tag;

/** Expected download size
 
 Set this property to the expected download size when running a SFNetworkOperation for downloading a binary content. If this property is not set, `SFNetworkOperation` will rely on the "Content-Length" in response header to properly invoke the `SFNetworkOperationProgressBlock` download progress block
 
 As of 180 release, salesforce content download API does not set "Content-Length" response header properly,  make sure set this property before start a download operation
 */
@property (nonatomic, assign) NSUInteger expectedDownloadSize;

/** Network timeout setting in seconds. Default value is 180 seconds
 */
@property (nonatomic, assign) NSTimeInterval operationTimeout;

/** Set to YES to enable automatic retry if operation failed to due network error. Default value is NO
 
 See `SFNetworkOperationErrorType` for details on the logic of detecting network error
 
 Because there is a slight chance that due to unstable connectivity operation can error out after server side
 receives the response and before operation can get a valid response back. You need to be careful when setting
 this property to YES and be aware of possible duplication requests if auto retry is turned on.
 */
@property (nonatomic, assign) BOOL retryOnNetworkError;

/** Maximum number of retries if operation failed due to network error. Default value is 0, i.e. no limit.
 */
@property (nonatomic, assign) NSUInteger maximumNumOfRetriesForNetworkError;

/** Set this property to enable SFNetworkOperation to read test data from a local file
 
 This feature is useful to simulate server side response using a local mock up data file for testing purpose.
 `[SFNetworkEngine supportLocalTestData]` needs to be set to YES for this property to take effect
 */
@property (nonatomic, copy) NSString *localTestDataPath;

/** Returns the HTTP method for this operation
 */
@property (nonatomic, readonly, copy) NSString *method;

/** Returns YES if use SSL
 */
@property (nonatomic, readonly, assign) BOOL useSSL;

/** Request URL Property
 */
@property (nonatomic, readonly, copy) NSString *url;

/** If the operation results in an error, this will hold the response error, otherwise it will be nil */
@property (nonatomic, readonly, strong) NSError *error;

/** Returns the operation response's status code.
 
 Returns 0 when the operation has not yet started and the response is not available.
 */
@property (nonatomic, readonly, assign) NSInteger statusCode;

/** Returns an uniqueIdentifer for this operation
 
 uniqueIdentifier is generated based on operation's method, url and parameters*/
@property (nonatomic, readonly, strong) NSString *uniqueIdentifier;

/**Delegate can be used to monitor operation status (complete, error, cancel or timeout) in lieu of using blocks
 */
@property (nonatomic, weak) id <SFNetworkOperationDelegate> delegate;

/**Set to YES to encrypt all downloaded content. Default value is YES*/
@property (nonatomic, assign) BOOL encryptDownloadedFile;

/**Set to YES if the operation requires an access token. Default value is YES*/
@property (nonatomic, assign) BOOL requiresAccessToken;

/** Custom HTTP headers that will be used by this operation
 
 CustomHeaders Value specified by this parameter will override value set by `[SFNetworkEngine customHeaders]`
 */
@property (nonatomic, copy) NSDictionary *customHeaders;

/**Set path to store downloaded content
 
 Path to store downloaded content. If this value is set, all content downloaded by this operation will be stored at the path specified. And  if `encryptDownloadedFile` is set to true, file content will be encrypted
 */
@property (nonatomic, copy) NSString *pathToStoreDownloadedContent;


/** Array of operation cancel blocks
 
 Each block in this array will be invoked when this operation is cancelled
 */
@property (nonatomic, readonly, strong) NSMutableArray *cancelBlocks;

/** Set value for the specified HTTP header
 
 @param value Header value. If value is nil, this method will remove value for the specified key from the headers
 @param key Header key
 */
- (void)setHeaderValue:(NSString *)value forKey:(NSString *)key;

/**Cache policy for this operation. Default value is NSURLRequestReloadIgnoringLocalCacheData*/
@property (nonatomic, assign) NSURLRequestCachePolicy cachePolicy;

/** HTTP headers for the response
 */
@property (nonatomic, readonly, strong) NSDictionary *responseHeaders;

///---------------------------------------------------------------
/// @name Post Method
///---------------------------------------------------------------
/** Set custom customized post data encoding support
 
 @param postDataEncodingHandler Block to be invoked when your HTTP Method is POST or PUT to translate request body into a custom string representation
 @param contentType Content type for the translated body content. For example, if your `postDataEncodingHandler` translates request body to JSON format, you will need to set contentType to "application/json"
 */
-(void)setCustomPostDataEncodingHandler:(SFNetworkOperationEncodingBlock)postDataEncodingHandler forType:(NSString*)contentType;

///---------------------------------------------------------------
/// @name Method for File Upload
///---------------------------------------------------------------
/** Attach file data as multipart/​form POST data
 
 This method can be used to upload binary file. A multi-part form data will be constructured based on the value passed in with format
 
 Content-Disposition: form-data; name=`name`; filename=`fileName` (if name is not nil)
 Content-Disposition: form-data; filename=`fileName` (if name is nil)
 Content-Type: `mimeType`
 `fileData`
 
 @param fileData File raw data
 @param paramName Parameter name to be used in the multi-part form data for this file. nil is accepted
 @param fileName File name to be used in the multi-part form data for this file
 @param mimeType File mimetype. Nil is accpeted. If nil is passed, 'multipart/form-data' will be used by default as the mimetype. Server side will use the fileName to figure out the proper mimetype for the file
 */
- (void)addPostFileData:(NSData *)fileData paramName:(NSString *)paramName fileName:(NSString *)fileName mimeType:(NSString *)mimeType;

- (void)addFile:(NSString *)file forKey:(NSString *)key;
///---------------------------------------------------------------
/// @name Block Methods
///---------------------------------------------------------------
/** Add block Handler for completion
 
 An operation can have multiple completion and error blocks attached to it.
 When the operation completes successfully and operation response is not a JSON array with single JSON error object, each registered completion block will be executed on a background thread.
 When operation errors out or operation response is a JSON array with single JSON error object, each registered error block will be executed on a background thread. Completion block will not be invoked when errorBlock is invoked
 @param completionBlock Completion block to be invoked when operation is completed successfully
 @param errorBlock Error block to be invoked when operation erros out or operation response is a JSON array with single JSON error object
 */
- (void)addCompletionBlock:(SFNetworkOperationCompletionBlock)completionBlock errorBlock:(SFNetworkOperationErrorBlock)errorBlock;


/** Add block Handler for cancel
 
 An operation can have multiple cancel blocks attached to it. When an operation is cancelled each registered block will be executed on a background thread.
 @param cancelBlock Error block to be invoked when operation is cancelled
 */
- (void)addCancelBlock:(SFNetworkOperationCancelBlock)cancelBlock;


/** Add Block Handler for tracking upload progress
 
 An operation can have multiple upload progress blocks attached to it. When upload process changes each registered block will be executed on a background thread
 @param uploadProgressBlock Block to be invoked when upload progress is changed
 */
- (void)addUploadProgressBlock:(SFNetworkOperationProgressBlock)uploadProgressBlock;

/** Add Block Handler for tracking download progress
 
 An operation can have multiple download progress blocks attached to it. When download process changes each registered block will be executed on a background thread
 @param downloadProgressBlock Block to be invoked when download progress is changed
 */
- (void)addDownloadProgressBlock:(SFNetworkOperationProgressBlock)downloadProgressBlock;


///---------------------------------------------------------------
/// @name Response Object Helper Methods
///---------------------------------------------------------------
/** Returns the downloaded data as a string.
 *
 * @return the response as a string; nil if the operation is in progress
 */
- (NSString *)responseAsString;

/** Returns the response as a JSON object.
 *
 * @return the response as an NSDictionary or an NSArray; nil if the operation is in progress or the response is not valid JSON
 */
- (id)responseAsJSON;

/** Returns the response as NSData.
 *
 * @return returns the response as raw data
 */
- (NSData *)responseAsData;

/** Returns the downloaded data as a UIImage.
 *
 * @return the respoonse as an image; nil if the operation is in progress or the response is not a valid image
 */
// - (UIImage *)responseAsImage;

@end
