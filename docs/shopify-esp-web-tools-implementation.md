# 在 Shopify 中上线 MeowKit 浏览器固件安装器

这是一套从固件构建到用户完成烧录的完整流程。第一次按顺序做；以后发布新版本，只需重复“发布固件”。

## 最终结构

```text
Shopify 页眉 Download
        ↓
/pages/download
        ↓
MeowKit 下载模式引导 + ESP Web Tools
        ↓
https://firmware.example.com/stable/manifest.json
        ↓
bootloader.bin / partitions.bin / boot_app0.bin / firmware.bin
```

Shopify 负责页面和品牌体验。固件 CDN 负责存放 manifest 与二进制文件。两者分开后，更新固件不需要重新发布 Shopify 主题。

## 开始前

准备：

- Shopify Online Store 2.0 主题的编辑权限
- 一个支持 HTTPS 和 CORS 的静态文件地址
- Windows、macOS 或 Linux 电脑
- Chrome 或 Edge
- 支持数据传输的 USB 线
- 一台可反复测试的 MeowKit

文中使用以下示例地址，请替换成自己的地址：

```text
商店：https://www.meowkit.com
固件：https://firmware.meowkit.com
```

## 第 1 步：构建固件

在项目根目录运行：

```powershell
pio run -e esp32s3box
```

如果 PowerShell 找不到 `pio`，使用 PlatformIO Core 的完整路径：

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32s3box
```

构建成功后需要四个文件：

| 发布文件 | 当前工程中的来源 | 烧录地址 |
| --- | --- | ---: |
| `bootloader.bin` | `.pio/build/esp32s3box/bootloader.bin` | `0x0000` |
| `partitions.bin` | `.pio/build/esp32s3box/partitions.bin` | `0x8000` |
| `boot_app0.bin` | Arduino-ESP32 框架的 `tools/partitions/boot_app0.bin` | `0xE000` |
| `firmware.bin` | `.pio/build/esp32s3box/firmware.bin` | `0x10000` |

本项目当前使用的 `boot_app0.bin` 位于：

```text
C:\Users\你的用户名\.platformio\packages\framework-arduinoespressif32@3.20006.221224\tools\partitions\boot_app0.bin
```

不要从另一个 Arduino-ESP32 版本或另一轮构建中混用文件。

## 第 2 步：建立发布目录

为每个版本创建不可变目录，同时维护一个 `stable` 目录：

```text
firmware/
├── releases/
│   └── 1.0.0/
│       ├── manifest.json
│       ├── bootloader.bin
│       ├── partitions.bin
│       ├── boot_app0.bin
│       └── firmware.bin
└── stable/
    ├── manifest.json
    ├── bootloader.bin
    ├── partitions.bin
    ├── boot_app0.bin
    └── firmware.bin
```

先把四个文件复制到 `releases/1.0.0/`。完成真机验证后，再把完全相同的文件发布到 `stable/`。

这样做有两个好处：稳定页面永远只有一个地址；出现问题时可以立即把 `stable` 指回上一个版本。

## 第 3 步：创建 manifest

在 `releases/1.0.0/manifest.json` 中写入：

```json
{
  "name": "MeowKit",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "new_install_improv_wait_time": 0,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "serialType": "cdc",
      "parts": [
        { "path": "bootloader.bin", "offset": 0 },
        { "path": "partitions.bin", "offset": 32768 },
        { "path": "boot_app0.bin", "offset": 57344 },
        { "path": "firmware.bin", "offset": 65536 }
      ]
    }
  ]
}
```

说明：

- `ESP32-S3` 限定正确芯片系列。
- `cdc` 对应 MeowKit 的原生 USB CDC 下载连接。
- `new_install_prompt_erase: true` 让用户在需要时选择完整擦除。
- `new_install_improv_wait_time: 0` 关闭 MeowKit 当前未实现的 Improv Serial 等待。
- manifest 中的偏移量是十进制，这是 JSON 示例的常见写法。

普通更新不要擦除整个 Flash，可以保留 NVS 中的设置。只有设备无法启动或重复更新失败时，才选择完整擦除。microSD 卡不属于内部 Flash，不会被烧录器改写。

## 第 4 步：托管固件

推荐顺序：

1. Cloudflare R2 + 自定义域名
2. Amazon S3 + CloudFront
3. GitHub Pages，适合早期验证

固件服务器必须：

- 使用 HTTPS
- 不需要登录
- 不返回网页跳转或下载确认页
- 正确返回 `.json` 和 `.bin`
- 允许 Shopify 商店跨域读取

至少设置以下响应头：

```http
Access-Control-Allow-Origin: https://www.meowkit.com
Access-Control-Allow-Methods: GET, HEAD, OPTIONS
Content-Type: application/json
```

`.bin` 文件应返回：

```http
Content-Type: application/octet-stream
```

如果商店同时使用根域名和 `www`，最简单的早期配置是：

```http
Access-Control-Allow-Origin: *
```

固件没有用户隐私数据，可以公开读取。不要允许公开写入。

在浏览器中直接打开下面的地址，确认看到 JSON 而不是 404、登录页或 HTML：

```text
https://firmware.meowkit.com/releases/1.0.0/manifest.json
```

## 第 5 步：先做独立验证页

在接触 Shopify 前，先确认固件包本身能烧录。创建一个临时 HTTPS 页面：

```html
<!doctype html>
<html lang="zh-CN">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>MeowKit Firmware Test</title>
    <script
      type="module"
      src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"
    ></script>
  </head>
  <body>
    <esp-web-install-button
      manifest="https://firmware.meowkit.com/releases/1.0.0/manifest.json"
    ></esp-web-install-button>
  </body>
</html>
```

用 Chrome 或 Edge 打开。让 MeowKit 进入下载模式：

1. 关闭 MeowKit。
2. 拔下 USB 线。
3. 按住 **BOOT**，同时打开电源。
4. 保持约 2 秒，然后松开 **BOOT**。
5. 用 USB 数据线连接电脑。
6. 点击安装按钮并选择新出现的串口。

烧录完成后拔线，关闭电源，再正常开机。只有这一步通过，才继续 Shopify 集成。

## 第 6 步：备份 Shopify 主题

在 Shopify 后台打开：

```text
Online Store → Themes
```

复制当前主题。所有修改先在副本上完成和预览，不要直接改线上主题。

## 第 7 步：添加 Shopify Section

打开主题副本：

```text
Online Store → Themes → … → Edit code
```

新建文件：

```text
sections/meowkit-firmware-installer.liquid
```

写入：

```liquid
{{ 'meowkit-firmware-installer.css' | asset_url | stylesheet_tag }}

<script
  type="module"
  src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"
></script>

<section class="mk-flasher" data-section-id="{{ section.id }}">
  <div class="mk-flasher__inner">
    <p class="mk-flasher__eyebrow">MEOWKIT FIRMWARE</p>
    <h1>{{ section.settings.heading | escape }}</h1>
    <p class="mk-flasher__intro">{{ section.settings.intro | escape }}</p>

    <div class="mk-flasher__steps" aria-label="进入下载模式">
      <div><span>1</span><p>关闭 MeowKit，并拔下 USB 线。</p></div>
      <div><span>2</span><p>按住 BOOT，同时打开电源。</p></div>
      <div><span>3</span><p>等待 2 秒。松开 BOOT，再连接电脑。</p></div>
    </div>

    {% if section.settings.manifest_url != blank %}
      <esp-web-install-button
        manifest="{{ section.settings.manifest_url | escape }}"
      >
        <button class="mk-flasher__button" slot="activate" type="button">
          连接 MeowKit
        </button>

        <p class="mk-flasher__warning" slot="unsupported">
          请在电脑上使用最新版 Chrome 或 Edge 打开此页面。
        </p>

        <p class="mk-flasher__warning" slot="not-allowed">
          固件安装器需要安全的 HTTPS 连接。
        </p>
      </esp-web-install-button>
    {% else %}
      <p class="mk-flasher__warning">固件地址尚未配置。</p>
    {% endif %}

    <p class="mk-flasher__note">
      更新完成前，请保持设备开机并连接 USB。
    </p>

    <details class="mk-flasher__help">
      <summary>电脑没有找到 MeowKit？</summary>
      <p>换一根确定支持数据传输的 USB 线，然后重新进入下载模式。</p>
      <p>直接连接电脑，不要使用 USB 扩展坞。</p>
      <p>关闭 PlatformIO 串口监视器或其他可能占用串口的软件。</p>
    </details>
  </div>
</section>

{% schema %}
{
  "name": "MeowKit firmware",
  "settings": [
    {
      "type": "text",
      "id": "heading",
      "label": "标题",
      "default": "让 MeowKit 保持最新"
    },
    {
      "type": "textarea",
      "id": "intro",
      "label": "简介",
      "default": "连接 MeowKit。剩下的交给我们。"
    },
    {
      "type": "url",
      "id": "manifest_url",
      "label": "Firmware manifest URL"
    }
  ],
  "presets": [
    {
      "name": "MeowKit firmware"
    }
  ]
}
{% endschema %}
```

先用固定的 `@10`，不要使用 `@latest`。准备正式量产后，建议把经过验证的 ESP Web Tools JavaScript 复制到自己的 CDN，并固定具体补丁版本，避免第三方 CDN 更新影响安装页。

## 第 8 步：添加页面样式

新建：

```text
assets/meowkit-firmware-installer.css
```

写入：

```css
.mk-flasher {
  min-height: 76vh;
  padding: 96px 24px;
  background: #080a0b;
  color: #f7f7f5;
}

.mk-flasher__inner {
  width: min(100%, 760px);
  margin: 0 auto;
  text-align: center;
}

.mk-flasher__eyebrow {
  margin: 0 0 18px;
  color: #bee700;
  font-size: 12px;
  font-weight: 700;
  letter-spacing: .16em;
}

.mk-flasher h1 {
  margin: 0;
  color: inherit;
  font-size: clamp(40px, 7vw, 72px);
  line-height: 1.05;
  letter-spacing: -.04em;
}

.mk-flasher__intro {
  max-width: 540px;
  margin: 24px auto 44px;
  color: #b9bdb8;
  font-size: 19px;
  line-height: 1.6;
}

.mk-flasher__steps {
  max-width: 560px;
  margin: 0 auto 38px;
  padding: 20px 30px;
  border: 1px solid #292d2b;
  border-radius: 22px;
  background: #111412;
  text-align: left;
}

.mk-flasher__steps div {
  display: grid;
  grid-template-columns: 32px 1fr;
  gap: 14px;
  align-items: center;
  padding: 10px 0;
}

.mk-flasher__steps span {
  display: grid;
  width: 30px;
  height: 30px;
  place-items: center;
  border-radius: 50%;
  background: #bee700;
  color: #080a0b;
  font-weight: 700;
}

.mk-flasher__steps p { margin: 0; }

.mk-flasher__button {
  min-width: 220px;
  padding: 16px 28px;
  border: 0;
  border-radius: 999px;
  background: #bee700;
  color: #080a0b;
  font: inherit;
  font-weight: 700;
  cursor: pointer;
  transition: transform .15s ease, background .15s ease;
}

.mk-flasher__button:hover {
  background: #d2ff00;
  transform: translateY(-1px);
}

.mk-flasher__button:focus-visible {
  outline: 3px solid #fff;
  outline-offset: 4px;
}

.mk-flasher__warning {
  color: #ffcc66;
  line-height: 1.5;
}

.mk-flasher__note {
  margin-top: 18px;
  color: #7f8581;
  font-size: 13px;
}

.mk-flasher__help {
  max-width: 560px;
  margin: 46px auto 0;
  padding-top: 22px;
  border-top: 1px solid #292d2b;
  color: #b9bdb8;
  text-align: left;
}

.mk-flasher__help summary {
  color: #f7f7f5;
  font-weight: 600;
  cursor: pointer;
}

@media (max-width: 749px) {
  .mk-flasher { padding: 64px 20px; }
  .mk-flasher__steps { padding: 18px 20px; }
}
```

## 第 9 步：创建 Download 模板

新建：

```text
templates/page.download.json
```

写入：

```json
{
  "sections": {
    "firmware_installer": {
      "type": "meowkit-firmware-installer",
      "settings": {
        "heading": "让 MeowKit 保持最新",
        "intro": "连接 MeowKit。剩下的交给我们。",
        "manifest_url": "https://firmware.meowkit.com/stable/manifest.json"
      }
    }
  },
  "order": ["firmware_installer"]
}
```

如果 Shopify 的代码编辑器不接受 URL 设置值，先把 `manifest_url` 留空并保存，再到主题编辑器中填写。

## 第 10 步：创建 Shopify 页面

打开：

```text
Online Store → Pages → Add page
```

填写：

```text
Title: Download
Theme template: page.download
Search engine listing URL: /pages/download
```

保存。页面正文可以留空，实际内容由刚创建的 Section 提供。

## 第 11 步：加入页眉

打开：

```text
Content → Menus → Main menu → Add menu item
```

填写：

```text
Name: Download
Link: Pages → Download
```

保存。页眉的 Download 现在应该打开：

```text
https://www.meowkit.com/pages/download
```

## 第 12 步：完成一次用户烧录

必须像第一次购买设备的用户一样测试：

1. 用电脑上的 Chrome 或 Edge 打开 Download 页面。
2. 关闭 MeowKit，并拔下 USB 线。
3. 按住 **BOOT**，同时打开电源。
4. 等待 2 秒，松开 **BOOT**。
5. 连接 USB 数据线。
6. 点击 **连接 MeowKit**。
7. 在浏览器弹窗中选择新出现的 USB 串口。
8. 选择安装。普通更新不要选择完整擦除。
9. 等待页面明确显示完成；不要中途关闭页面、设备或拔线。
10. 拔下 USB 线，关闭 MeowKit，再正常开机。
11. 在设备 About 页面确认版本号。

如果下载模式下屏幕不亮，应在网页上明确写出“这是正常的”。

## 第 13 步：验收失败路径

上线前逐项验证：

- Windows 11 + Chrome
- Windows 11 + Edge
- 当前版 macOS + Chrome
- Ubuntu LTS + Chrome
- ChromeOS
- Safari、Firefox、iPhone 和 iPad 显示明确的不支持说明
- 充电线会引导用户更换数据线
- 使用 USB 扩展坞失败时有明确提示
- PlatformIO 占用串口时有明确提示
- 更新中途拔线后，可以重新进入下载模式恢复
- 完整擦除后可以从空白 Flash 恢复
- 普通更新后 NVS 设置仍然存在
- microSD 卡文件未变化
- 烧录错误型号或损坏 manifest 时不会静默继续

## 第 14 步：发布新版本

以后每次发布按这个顺序：

1. 更新固件中可见的版本号。
2. 执行一次干净的 PlatformIO 构建。
3. 创建新的不可变版本目录，例如 `releases/1.1.0/`。
4. 从同一次构建复制四个文件。
5. 将 manifest 的 `version` 改为 `1.1.0`。
6. 计算并保存发布文件的 SHA-256。
7. 在测试地址上烧录至少两台设备。
8. 验证普通更新、完整恢复和中断恢复。
9. 将通过测试的内容发布到 `stable/`。
10. 用 Shopify 正式 Download 页面再烧录一次。
11. 最后发布更新说明。

Windows 生成校验值：

```powershell
Get-FileHash .\bootloader.bin -Algorithm SHA256
Get-FileHash .\partitions.bin -Algorithm SHA256
Get-FileHash .\boot_app0.bin -Algorithm SHA256
Get-FileHash .\firmware.bin -Algorithm SHA256
```

不要覆盖已经发布的 `releases/1.0.0/`。版本目录必须保持不可变，只有 `stable/` 可以移动。

## 常见问题

### 点击按钮没有弹出串口选择

确认使用桌面版 Chrome 或 Edge，并通过 HTTPS 打开页面。Web Serial 的设备选择必须由用户点击触发，不能在页面加载时自动弹出。

### 能看到串口，但连接失败

关闭 PlatformIO Monitor、Arduino Serial Monitor 和其他可能占用端口的软件，然后重新进入下载模式。

### 一直停在连接阶段

最常见原因是 USB 线只能充电。换线并直接连接电脑 USB 接口。

### 页面能打开，但 manifest 加载失败

在开发者工具的 Network 和 Console 中检查：

- manifest URL 是否返回 200
- 返回内容是否真的是 JSON
- 固件 URL 是否返回 200
- 是否出现 CORS 错误
- 是否发生 301/302 跳转到登录页或下载页

### 烧录完成但无法启动

检查四个文件是否来自同一次构建，地址是否正确；然后选择完整擦除并重新安装。仍然失败时，回退到上一个已验证的版本。

## 正式上线建议

- 页面只提供一个主按钮：**连接 MeowKit**。
- 把“完整擦除”放在恢复路径，不要作为默认操作。
- 在设备机身、包装和 About 页面使用同一个短链接二维码。
- 固件 CDN 开启版本化、日志和回滚。
- ESP Web Tools 固定具体版本；量产后自托管脚本。
- 不在 Shopify 中保存私钥或 CDN 写入凭证。
- 不用 iframe 嵌入外部烧录网站。直接在 Shopify 页面加载组件，用户路径更短，Web Serial 权限也更简单。

