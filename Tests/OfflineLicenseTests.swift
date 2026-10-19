import Foundation
import CryptoKit

@main
struct OfflineLicenseTests {
    static func main() throws {
        let issuer = Curve25519.Signing.PrivateKey()
        let publicKey = issuer.publicKey.rawRepresentation.base64EncodedString()
        let device = "11111111-AAAA-3333-BBBB-555555555555"
        let current = Int64(Date().timeIntervalSince1970)
        func token(device: String? = nil, expiry: Int64? = nil, nonce: String = "offline-proof", scope: String = "trollspeed-rebuild", corrupt: Bool = false) throws -> String {
            let object: [String: Any] = ["version":1,"license_id":"fixture-only","device_id":device ?? "11111111-AAAA-3333-BBBB-555555555555","issued_at":current-1,"expires_at":expiry ?? (current + 3600),"nonce":nonce,"scope":scope]
            let payload = try JSONSerialization.data(withJSONObject: object, options: .sortedKeys)
            var signature = try issuer.signature(for: payload)
            if corrupt { signature[0] ^= 1 }
            let envelope = try JSONSerialization.data(withJSONObject: ["payload":payload.base64EncodedString(),"signature":signature.base64EncodedString()])
            return "TSOFF1." + envelope.base64EncodedString().replacingOccurrences(of: "+", with: "-").replacingOccurrences(of: "/", with: "_").replacingOccurrences(of: "=", with: "")
        }
        var checks = 0
        func reject(_ code: String, installation: String? = nil) throws {
            do {
                _ = try LicenseClient.verifyOffline(code, publicKeyBase64: publicKey, installation: installation ?? device)
            } catch { checks += 1; return }
            throw NSError(domain: "OfflineVerificationUnexpectedAcceptance", code: checks)
        }
        let license = try LicenseClient.verifyOffline(token(), publicKeyBase64: publicKey, installation: device)
        guard license.device_id == device else { throw LicenseError.wrongInstallation };checks += 1
        try reject(token(), installation: "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE")
        try reject(token(expiry: current-1))
        try reject(token(corrupt: true))
        try reject(token(nonce: "not-an-offline-code"))
        try reject(token(scope: "wrong-product"))
        try reject("TSOFF1.invalid")
        if CommandLine.arguments.count == 2 {
            let data = try Data(contentsOf: URL(fileURLWithPath: CommandLine.arguments[1]))
            let fixture = try JSONSerialization.jsonObject(with: data) as! [String:String]
            _ = try LicenseClient.verifyOffline(fixture["code"]!, publicKeyBase64: fixture["public_key"]!, installation: fixture["device"]!)
            checks += 1
        }
        print("\(checks) offline license checks passed with real CryptoKit signature verification")
    }
}
