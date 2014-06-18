//
//  SFNetworkCoordinator.h
//  SalesforceNetworkSDK
//
//  Created by Qingqing Liu on 3/15/13.
//  Copyright (c) 2013 salesforce.com. All rights reserved.
//

#import <Foundation/Foundation.h>

/** Main class to encapulate information for making network request

 Information include where to make the request, as what user, in which org and with what access token
 
 */
@interface SFNetworkCoordinator : NSObject

@property (strong) NSString *host;
@property (strong) NSString *organizationId;
@property (strong) NSString *userId;
@property (strong) NSNumber *portNumber;
@property (strong) NSNumber *sslPortNumber;
@property (strong) NSString *apiUrl;

/** Access token */
@property (strong) NSString *accessToken;
@end