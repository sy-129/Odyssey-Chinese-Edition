#import <UIKit/UIKit.h>
#import "TrollSpeedRebuild-Swift.h"
#include "HUDProcess.hpp"
#include <cmath>
#include "SettingsStore.hpp"
#include "SettingLabels.hpp"
@interface MainController:UIViewController<UITableViewDataSource,UITableViewDelegate>
@end
@implementation MainController {
 UITextField* _server;UITextField* _key;UILabel* _status;UITableView* _table;
 NSMutableDictionary<NSString*,NSNumber*>* _settings;NSArray<NSString*>* _names;
}
-(UITextField*)field:(NSString*)placeholder secure:(BOOL)secure{
 UITextField* field=[UITextField new];field.placeholder=placeholder;field.borderStyle=UITextBorderStyleRoundedRect;field.secureTextEntry=secure;field.autocapitalizationType=UITextAutocapitalizationTypeNone;field.autocorrectionType=UITextAutocorrectionTypeNo;return field;
}
-(UIButton*)button:(NSString*)title action:(SEL)action{
 UIButton* button=[UIButton buttonWithType:UIButtonTypeSystem];[button setTitle:title forState:UIControlStateNormal];[button addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];return button;
}
-(void)viewDidLoad{
 [super viewDidLoad];self.title=@"TrollSpeed";self.view.backgroundColor=UIColor.systemBackgroundColor;
 _settings=[ts::settingsDictionary(ts::loadSettings()) mutableCopy];_names=[_settings.allKeys sortedArrayUsingSelector:@selector(compare:)];
 _server=[self field:@"授权服务器 https://…" secure:NO];_server.keyboardType=UIKeyboardTypeURL;_server.text=[NSUserDefaults.standardUserDefaults stringForKey:@"authorizationServer"]?:@"";
 _key=[self field:@"授权码" secure:YES];_status=[UILabel new];_status.numberOfLines=0;_status.font=[UIFont systemFontOfSize:13];_status.text=@"先配置授权服务器并激活，再启动悬浮层。设置修改后立即保存。";
 UIStackView* buttons=[[UIStackView alloc] initWithArrangedSubviews:@[[self button:@"激活 / 续期" action:@selector(activate)],[self button:@"启动悬浮层" action:@selector(start)],[self button:@"停止悬浮层" action:@selector(stop)],[self button:@"复制诊断" action:@selector(diagnostics)]]];buttons.distribution=UIStackViewDistributionFillEqually;
 UIStackView* header=[[UIStackView alloc] initWithArrangedSubviews:@[_server,_key,buttons,_status]];header.axis=UILayoutConstraintAxisVertical;header.spacing=10;header.translatesAutoresizingMaskIntoConstraints=NO;[self.view addSubview:header];
 _table=[[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleInsetGrouped];_table.dataSource=self;_table.delegate=self;_table.translatesAutoresizingMaskIntoConstraints=NO;[self.view addSubview:_table];
 [NSLayoutConstraint activateConstraints:@[[header.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:12],[header.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],[header.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],[_table.topAnchor constraintEqualToAnchor:header.bottomAnchor constant:8],[_table.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],[_table.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],[_table.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]]];
}
-(void)activate{
 [self.view endEditing:YES];[NSUserDefaults.standardUserDefaults setObject:_server.text?:@"" forKey:@"authorizationServer"];
 _status.text=@"正在连接授权服务器…";__weak MainController* weak=self;
 [TSLicenseBridge activate:_server.text?:@"" key:_key.text?:@"" completion:^(BOOL success,NSString* message){(void)success;MainController* strong=weak;if(strong)strong->_status.text=message;}];
}
-(void)start{
 if(![TSLicenseBridge hasValidLease]){_status.text=@"尚无有效授权，请先激活。";return;}
 auto result=ts::spawnHUDMode(true);_status.text=[NSString stringWithUTF8String:result.message.c_str()];
}
-(void)stop{auto result=ts::spawnHUDMode(false);_status.text=[NSString stringWithUTF8String:result.message.c_str()];}
-(void)diagnostics{
 NSString* report=[NSString stringWithContentsOfFile:@"/var/mobile/Library/Caches/local.trollspeed.rebuild.status.json" encoding:NSUTF8StringEncoding error:nil];
 NSString* startup=[NSString stringWithContentsOfFile:@"/var/mobile/Library/Caches/local.trollspeed.rebuild.log" encoding:NSUTF8StringEncoding error:nil];
 UIPasteboard.generalPasteboard.string=[NSString stringWithFormat:@"启动记录：%@\n运行诊断：%@",startup?:@"尚无启动记录",report?:@"尚无运行记录"];
 _status.text=@"已复制运行诊断，可用于定位启动、授权或数据读取问题。";
}
-(NSInteger)tableView:(UITableView*)table numberOfRowsInSection:(NSInteger)section{(void)table;(void)section;return _names.count;}
-(UITableViewCell*)tableView:(UITableView*)table cellForRowAtIndexPath:(NSIndexPath*)index{
 (void)table;UITableViewCell* cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:nil];NSString* name=_names[index.row];NSNumber* value=_settings[name];cell.textLabel.text=[NSString stringWithUTF8String:ts::settingLabel(name.UTF8String)];cell.textLabel.font=[UIFont systemFontOfSize:13];
 if(CFGetTypeID((__bridge CFTypeRef)value)==CFBooleanGetTypeID()){
  UISwitch* toggle=[UISwitch new];toggle.on=value.boolValue;toggle.tag=index.row;[toggle addTarget:self action:@selector(toggle:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=toggle;
 }else cell.detailTextLabel.text=value.stringValue;
 return cell;
}
-(void)toggle:(UISwitch*)sender{_settings[_names[sender.tag]]=@(sender.on);ts::saveSettings(ts::settingsFromDictionary(_settings));}
-(void)tableView:(UITableView*)table didSelectRowAtIndexPath:(NSIndexPath*)index{
 [table deselectRowAtIndexPath:index animated:YES];NSString* name=_names[index.row];NSNumber* value=_settings[name];if(CFGetTypeID((__bridge CFTypeRef)value)==CFBooleanGetTypeID())return;
 UIAlertController* alert=[UIAlertController alertControllerWithTitle:[NSString stringWithUTF8String:ts::settingLabel(name.UTF8String)] message:@"输入数值" preferredStyle:UIAlertControllerStyleAlert];[alert addTextFieldWithConfigurationHandler:^(UITextField* field){field.text=value.stringValue;field.keyboardType=UIKeyboardTypeNumbersAndPunctuation;}];
 [alert addAction:[UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleCancel handler:nil]];
 [alert addAction:[UIAlertAction actionWithTitle:@"保存" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){(void)action;NSScanner* scanner=[NSScanner scannerWithString:alert.textFields.firstObject.text?:@""];double number=0;if(![scanner scanDouble:&number]||!scanner.isAtEnd||!std::isfinite(number))return;self->_settings[name]=@(number);ts::saveSettings(ts::settingsFromDictionary(self->_settings));[self->_table reloadRowsAtIndexPaths:@[index] withRowAnimation:UITableViewRowAnimationNone];}]];[self presentViewController:alert animated:YES completion:nil];
}
@end
